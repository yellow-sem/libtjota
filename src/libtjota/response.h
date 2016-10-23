#ifndef RESPONSE_H
#define RESPONSE_H

#include <stdbool.h>

typedef struct
{
    char *command;
    char *ident;
    char *value;
    bool ok;
} tm_response;

tm_response *tm_response_decode(char *data);
void tm_response_free(tm_response *response);

#endif
