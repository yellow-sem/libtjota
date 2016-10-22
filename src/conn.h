#ifndef CONN_H
#define CONN_H

#include <netdb.h>

#include "config.h"

struct tm_conn
{
    struct hostent *hostent;
    int sockfd;
};

struct tm_conn *tm_conn_new();
void tm_conn_free(struct tm_conn *conn);

struct tm_conn *tm_conn_open(struct tm_config *config);
void tm_conn_close(struct tm_conn *conn);

char *tm_conn_read(struct tm_conn *conn);
void tm_conn_write(struct tm_conn *conn, const char *data);

#endif
