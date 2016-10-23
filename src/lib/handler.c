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
    return handler;
}

void tm_handler_free(tm_handler *handler)
{
    if (handler->regex_c != NULL) tm_regex_free(handler->regex_c);
    free(handler);
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
        handler->handle(argc, argv);
    }

    tm_regex_match_free(&argc, &argv);

    return match;
}
