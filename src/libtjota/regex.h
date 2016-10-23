#ifndef REGEX_H
#define REGEX_H

#include <sys/types.h>
#include <stdbool.h>
#include <regex.h>

regex_t *tm_regex_compile(const char *regex_string);
void tm_regex_free(regex_t *regex);

char *tm_regex_substr(const char *string, int a, int b);
bool tm_regex_match(regex_t *regex, const char *string,
                    int *argc, char ***argv);
void tm_regex_match_free(int *argc, char ***argv);

#endif
