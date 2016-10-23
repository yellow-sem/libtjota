#include "request.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "string.h"

struct tm_request *tm_request_new()
{
    struct tm_request *request = malloc(sizeof(struct tm_request));
    request->command = NULL;
    request->ident = tm_string_random(10);
    request->argc = 0;
    request->argv = NULL;
    return request;
}

struct tm_request *tm_request_new_command(char *command)
{
    struct tm_request *request = tm_request_new();
    request->command = command;
    return request;
}

struct tm_request *tm_request_new_command_args(char *command, int argc, ...)
{
    va_list args;
    va_start(args, argc);

    char **argv = malloc(sizeof(char *) * argc);
    int i;
    for (i = 0; i < argc; i++) {
        argv[i] = va_arg(args, char *);
    }

    struct tm_request *request = tm_request_new_command(command);
    request->argc = argc;
    request->argv = argv;

    va_end(args);

    return request;
}

void tm_request_free(struct tm_request *request)
{
    free(request->ident);

    if (request->argv != NULL) {
        free(request->argv);
    }

    free(request);
}

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
