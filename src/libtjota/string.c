#include "string.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

char *tm_string_join(const char *base, int argc, char **argv) {
    int i;

    bool first = true;

    int length = 0;
    for (i = 0; i < argc; i++) {
        char *arg = argv[i];
        if (arg != NULL) {
            if (first) {
                first = false;
            } else {
                length += strlen(base);
            }

            length += strlen(arg);
        }
    }

    char *string = (char*) malloc((sizeof(char) * length) + 1);

    string[0] = '\0';

    first = true;

    for (i = 0; i < argc; i++) {
        char *arg = argv[i];
        if (arg != NULL) {
            if (first) {
                first = false;
            } else {
                strcat(string, base);
            }

            strcat(string, arg);
        }
    }

    return string;
}

char *tm_string_random(int length)
{
    char *string = (char*) malloc((sizeof(char) * length) + 1);
    static const char charset[] = "abcdef0123456789";

    int i;
    for (i = 0; i < length; i++) {
        string[i] = charset[rand() % (int) (sizeof(charset) - 1)];
    }

    string[length] = '\0';

    return string;
}
