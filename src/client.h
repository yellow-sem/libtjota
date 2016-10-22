#ifndef CLIENT_H
#define CLIENT_H

#include <netdb.h>

#include "config.h"

struct tm_handler
{
};

struct tm_client
{
    struct hostent *hostent;
    int sockfd;
};

struct tm_client *tm_client_new();
void tm_client_free(struct tm_client *client);

struct tm_client *tm_client_connect(struct tm_config *config);
void tm_client_disconnect(struct tm_client *client);

void tm_client_set_handler(struct tm_client *client,
                           struct tm_handler *handler);

#endif
