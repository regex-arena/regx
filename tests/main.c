#include "../lib/regx.h"
#include <stdio.h>
#include <string.h>


int main(void) {
    char* regexString = "a*.b";
    struct Arena arena = arenaAllocate(sizeof(struct Pattern)*strlen(regexString));
    struct Pattern* pattern = regxComp(&arena, regexString);
    char* matchString = "aaazb";
    regxRun(pattern, matchString);
    printf("hello\n");
}
