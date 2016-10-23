#ifndef HANDLER_H
#define HANDLER_H

#include <stdbool.h>

#include "regex.h"

typedef struct
{
    const char *regex_s;
    regex_t *regex_c;
    void (*handle)(int argc, char **argv);
} tm_handler;

tm_handler *tm_handler_new();
void tm_handler_free(tm_handler *handler);

bool tm_handler_handle(tm_handler *handler, char *data);

#endif
