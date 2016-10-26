#include "handler.h"

#include <stdbool.h>
#include <stdlib.h>

#include "regex.h"

tm_handler *tm_handler_new()
{
    tm_handler *handler = malloc(sizeof(tm_handler));
    handler->regex_s = NULL;
    handler->regex_c = NULL;
    handler->handle = NULL;
    handler->data = NULL;
    return handler;
}

tm_handler *tm_handler_new_regex(const char *regex_s,
                                 void (*handle)(int argc, char **argv,
                                                void *data),
                                 void *data)
{
    tm_handler *handler = tm_handler_new();
    handler->regex_s = regex_s;
    handler->handle = handle;
    handler->data = data;
    return handler;
}

void tm_handler_free(tm_handler *handler)
{
    if (handler->regex_c != NULL) tm_regex_free(handler->regex_c);
    free(handler);
}

void tm_handler_free_all(tm_handler **handlers)
{
    int i = 0;
    tm_handler *handler = handlers[i];
    while (handler != NULL) {
        tm_handler_free(handler);
        handler = handlers[++i];
    }
}

bool tm_handler_handle(tm_handler *handler, char *data)
{
    if (handler->regex_c == NULL) {
        handler->regex_c = tm_regex_compile(handler->regex_s);
    }

    int argc;
    char **argv;

    bool match = tm_regex_match(handler->regex_c, data, &argc, &argv);

    if (match) {
        handler->handle(argc, argv, handler->data);
    }

    tm_regex_match_free(&argc, &argv);

    return match;
}

bool tm_handler_handle_all(tm_handler **handlers, char *data)
{
    bool match = false;

    int i = 0;
    tm_handler *handler = handlers[i];
    while (handler != NULL) {
        match |= tm_handler_handle(handler, data);
        handler = handlers[++i];
    }

    return match;
}
