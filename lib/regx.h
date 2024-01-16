#ifndef __REGX_H__
#define __REGX_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <arena.h>
#include <stdint.h>

enum Type {
    ITEM,
    PATTERN
};

enum Count {
    ONCE,
    OPTIONAL,
    ZERO_OR_MORE
};

union Element {
    struct Pattern* pattern;
    struct {
        // Bitwize character representation
        uint64_t high;
        uint64_t low;
    } item;
};

struct Pattern {
    struct Pattern* next;
    union Element value;
    enum Type type;
    enum Count count;
};

struct Pattern*  regxComp(struct Arena* arena, char* pattern);
int regxRun(struct Pattern* pattern, char* string);

#ifdef __cplusplus
}
#endif
#endif
