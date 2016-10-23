#include "response.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "regex.h"

tm_response *tm_response_decode(char *data)
{
    tm_response *response = NULL;

    regex_t *regex = tm_regex_compile("([^ ]*) ([^ ]*) ([^ ]*)");

    int argc;
    char **argv;

    bool match = tm_regex_match(regex, data, &argc, &argv);

    if (match) {
        response = malloc(sizeof(tm_response));

        response->command = malloc(strlen(argv[1]) + 1);
        response->ident = malloc(strlen(argv[2]) + 1);
        response->value = malloc(strlen(argv[3]) + 1);

        strcpy(response->command, argv[1]);
        strcpy(response->ident, argv[2]);
        strcpy(response->value, argv[3]);

        if (strcmp(response->value, "ok") == 0) {
            response->ok = true;
        }

        if (strcmp(response->value, "err") == 0) {
            response->ok = false;
        } else {
            response->ok = true;
        }
    }

    tm_regex_match_free(&argc, &argv);
    tm_regex_free(regex);

    return response;
}

void tm_response_free(tm_response *response)
{
    free(response->command);
    free(response->ident);
    free(response->value);
    free(response);
}
