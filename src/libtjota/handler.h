#ifndef HANDLER_H
#define HANDLER_H

#include <stdbool.h>

#include "regex.h"

typedef struct
{
    const char *regex_s;
    regex_t *regex_c;
    void (*handle)(int argc, char **argv, void *data);
    void *data;
} tm_handler;

tm_handler *tm_handler_new();
tm_handler *tm_handler_new_regex(const char *regex_s,
                                 void (*handle)(int argc, char **argv,
                                                void *data),
                                 void *data);
void tm_handler_free(tm_handler *handler);
void tm_handler_free_all(tm_handler **handlers);

bool tm_handler_handle(tm_handler *handler, char *data);
bool tm_handler_handle_all(tm_handler **handlers, char *data);

#endif
