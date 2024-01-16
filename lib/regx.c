#include "regx.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static void set(struct Pattern* pattern, char value);

// Subquery used to detect if incorrect matches for (
struct PatternWrapper {
    struct Pattern* pattern;
    int index;
    char subquery;
};

static struct PatternWrapper compile(struct Arena* arena, char* pattern) {
    struct Pattern *head, *start;
    // Start and head points to one before the first value
    head = start = arenaAssignBytes(arena, sizeof(struct Pattern));
    if (head == NULL) {
        struct PatternWrapper fail = {
            NULL,
            0,
            0
        };
        return fail;
    }
    int len = strlen(pattern);
    int charGroup = 0;
    int escaped = 0;
    int i;
    for (i = 0; i < len; i++) {
        if (charGroup) {
            continue;
        }
        if (escaped) {
            escaped = !escaped;
        }
        switch (pattern[i]) {
            case '[': {
                break;
            }
            case '(': {
                // pattern + i + 1 gets first character after current character
                struct PatternWrapper sub = compile(arena, pattern + i + 1);
                if (sub.pattern == NULL) {
                    return sub;
                } else if (sub.subquery != ')') {
                    // Occours if end of pattern occours while parsing subquery
                    fprintf(stderr, "unmatched ')' in pattern");
                    errno = EBADMSG;
                    struct PatternWrapper err = {NULL, 0, 0};
                    return err;
                }
                // 1 comes from the added 1 when making recursive call
                i += 1+sub.index;
                break;
            }
            case ')': {
                struct PatternWrapper ret;
                ret.pattern = start->next;
                ret.index = i;
                ret.subquery = ')';
                return ret;
                break;
            }
            case '\\': {
                escaped = true;
                break;
            }
            case '.': {
                struct Pattern* new = arenaAssignBytes(arena, sizeof(struct Pattern));
                if (new == NULL) {
                    struct PatternWrapper fail = {
                        NULL,
                        0,
                        0
                    };
                    return fail;
                }
                new->type = ITEM;
                new->value.item.low = UINT64_MAX;
                new->value.item.high = UINT64_MAX;
                head->next = new;
                new->next = NULL;
                head = new;
                break;
            }
            case '+': {
                if (head->count != ONCE) {
                    fprintf(stderr, "Unable to change match count - already assigned");
                    errno = EBADMSG;
                    struct PatternWrapper fail = { NULL, 0, 0 };
                    return fail;
                }
                struct Pattern* new = arenaAssignBytes(arena, sizeof(struct Pattern));
                new->count = ZERO_OR_MORE;
                new->next = NULL;
                new->value = head->value;
                new->type = head->type;
                head->next = new;
                head = new;
                break;
            }
            case '*': {
                if (head->count != ONCE) {
                    fprintf(stderr, "Unable to change match count - already assigned");
                    errno = EBADMSG;
                    struct PatternWrapper fail = { NULL, 0, 0 };
                    return fail;
                }
                head->count = ZERO_OR_MORE;
                break;
            }
            case '?': {
                if (head->count != ONCE) {
                    fprintf(stderr, "Unable to change match count - already assigned");
                    errno = EBADMSG;
                    struct PatternWrapper fail = { NULL, 0, 0 };
                    return fail;
                }
                head->count = OPTIONAL;
                break;
            }
            default: {
                struct Pattern* new = arenaAssignBytes(arena, sizeof(struct Pattern));
                if (new == NULL) {
                    struct PatternWrapper fail = {
                        NULL,
                        0,
                        0
                    };
                    return fail;
                }
                new->type = ITEM;
                set(new, pattern[i]);
                head->next = new;
                new->next = NULL;
                head = new;
                break;
            }
        }
    }
    struct PatternWrapper ret = {
        start->next,
        i,
        0
    };
    return ret;
}

static void set(struct Pattern* pattern, char value) {
    if (value >= 64) {
        pattern->value.item.high |= (1<<(value%64));
    } else {
        pattern->value.item.low |= (1<<value);
    }
}

struct Pattern* regxComp(struct Arena* arena, char* pattern) {
    struct PatternWrapper wrapper = compile(arena, pattern);
    if (wrapper.pattern == NULL) {
        // errno set by compile function
        return NULL;
    } else if (wrapper.subquery == ')') {
        // TODO: check for better errono values
        errno = EBADMSG;
        fprintf(stderr, "Invalid regex pattern, missing ')'");
        return NULL;
    }
    return wrapper.pattern;
}

static int match(struct Pattern* pattern, char test) {
    if (test >= 64) {
        return pattern->value.item.high & (1<<(test%64));
    } else {
        return pattern->value.item.high & (1<<test);
    }
}

int regxRun(struct Pattern* pattern, char* string) {
    int index = 0;
    while (pattern != NULL) {
        switch (pattern->count) {
            case ONCE: {
                if (pattern->type == ITEM) {
                    if (match(pattern, string[index]) == 0) {
                        return -1;
                    }
                    index++;
                } else {
                    int ret = regxRun(pattern->value.pattern, string + index);
                    if (ret != -1) {
                        index += ret;
                    } else {
                        return -1;
                    }
                }
                break;
            }
            case OPTIONAL: {
                break;
            }
            case ZERO_OR_MORE: {
                break;
            }
        }
        pattern = pattern->next;
    }
    return 0;
}
