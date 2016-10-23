#ifndef HANDLER_H
#define HANDLER_H

#include <stdbool.h>

#include "regex.h"

struct tm_handler
{
    const char *regex_s;
    regex_t *regex_c;
    void (*handle)(int argc, char **argv);
};

struct tm_handler *tm_handler_new();
void tm_handler_free(struct tm_handler *handler);

bool tm_handler_handle(struct tm_handler *handler, char *data);

#endif
