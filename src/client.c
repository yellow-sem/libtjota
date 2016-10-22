#include "client.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "log.h"
#include "config.h"

struct tm_client *tm_client_new()
{
    return malloc(sizeof(struct tm_client));
}

void tm_client_free(struct tm_client *client)
{
    free(client);
}

struct tm_client *tm_client_connect(struct tm_config *config)
{
    struct hostent *hostent = gethostbyname(config->host);
    if (hostent == NULL) {
        tm_log_write(LOG_ERR,
                     "Could not resolve '%s'",
                     config->host);
        return NULL;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        tm_log_write(LOG_ERR,
                     "Could not obtain socket file descriptor");
        return NULL;
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));

    memcpy(&servaddr.sin_addr.s_addr,
           hostent->h_addr,
           hostent->h_length);

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(config->port);

    if (connect(sockfd,
                (struct sockaddr *) &servaddr,
                sizeof(servaddr)) < 0) {
        tm_log_write(LOG_ERR,
                     "Could not connect to %s:%d",
                     config->host,
                     config->port);
        return NULL;
    }

    struct tm_client *client = tm_client_new();
    client->hostent = hostent;
    client->sockfd = sockfd;
    return client;
}

void tm_client_disconnect(struct tm_client *client)
{
    close(client->sockfd);
    tm_client_free(client);
}

void tm_client_set_handler(struct tm_client *client,
                           struct tm_handler *handler)
{
}
