#ifndef CONN_H
#define CONN_H

#include <netdb.h>

#include "config.h"

typedef struct
{
    struct hostent *hostent;
    int sockfd;
} tm_conn;

tm_conn *tm_conn_new();
void tm_conn_free(tm_conn *conn);

tm_conn *tm_conn_open(tm_config *config);
void tm_conn_close(tm_conn *conn);

bool tm_conn_select(tm_conn *conn);
char *tm_conn_read(tm_conn *conn);
void tm_conn_write(tm_conn *conn, const char *data);

#endif
