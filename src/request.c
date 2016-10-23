#include "request.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "string.h"

char *tm_request_encode(struct tm_request *request)
{
    int argc = 2 + request->argc;
    char **argv = malloc(sizeof(char *) * argc);

    argv[0] = request->command;
    argv[1] = request->ident;

    int i;

    for (i = 0; i < request->argc; i++) {
        char *arg = request->argv[i];

        char *argq = malloc(2 + strlen(arg) + 1);
        sprintf(argq, "'%s'", arg);

        argv[2 + i] = argq;
    }

    char *data = tm_string_join(" ", argc, argv);

    for (i = 0; i < request->argc; i++) {
        free(argv[2 + i]);
    }

    free(argv);

    return data;
}
