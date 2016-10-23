#ifndef RESPONSE_H
#define RESPONSE_H

#include <stdbool.h>

struct tm_response
{
    char *command;
    char *ident;
    char *value;
    bool ok;
};

struct tm_response *tm_response_decode(char *data);
void tm_response_free(struct tm_response *response);

#endif
