#include "regex.h"

#include <sys/types.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

regex_t *tm_regex_compile(const char *regex_string)
{
    regex_t *regex = malloc(sizeof(regex_t));
    regcomp(regex, regex_string, REG_EXTENDED);
    return regex;
}

void tm_regex_free(regex_t *regex)
{
    regfree(regex);
    free(regex);
}

char *tm_regex_substr(const char *string, int a, int b)
{
    int length = b - a;
    char *ext = malloc(sizeof(char) * (length + 1));

    memcpy(ext, &string[a], length);
    ext[length] = '\0';
    return ext;
}

bool tm_regex_match(regex_t *regex, const char *string,
                    int *argc, char ***argv)
{

    size_t ngroups = regex->re_nsub + 1;
    regmatch_t *groups = calloc(ngroups, sizeof(regmatch_t));
    regexec(regex, string, ngroups, groups, 0);
    *argv = malloc(sizeof(char*) * ngroups);

    bool result = false;

    int i;
    char *part;
    for (i = 0; i < ngroups; i++) {
        part = tm_regex_substr(string, groups[i].rm_so, groups[i].rm_eo);
        (*argv)[i] = part;

        if (!result && strlen(part) > 0) {
            result = true;
        }
    }

    *argc = ngroups;

    free(groups);
    return result;
}

void tm_regex_match_free(int *argc, char ***argv)
{
    int i;
    for (i = 0; i < *argc; i++) {
        free((*argv)[i]);
    }

    free(*argv);
}
