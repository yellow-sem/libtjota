#include "conn.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "log.h"
#include "config.h"

struct tm_conn *tm_conn_new()
{
    return malloc(sizeof(struct tm_conn));
}

void tm_conn_free(struct tm_conn *conn)
{
    free(conn);
}

struct tm_conn *tm_conn_open(struct tm_config *config)
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

    struct tm_conn *conn = tm_conn_new();
    conn->hostent = hostent;
    conn->sockfd = sockfd;
    return conn;
}

void tm_conn_close(struct tm_conn *conn)
{
    close(conn->sockfd);
    tm_conn_free(conn);
}

bool tm_conn_select(struct tm_conn *conn)
{
    fd_set fd_set;
    FD_ZERO(&fd_set);
    FD_SET(conn->sockfd, &fd_set);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 1000;

    return select(conn->sockfd + 1, &fd_set, NULL, NULL, &timeout) > 0;
}

char *tm_conn_read(struct tm_conn *conn)
{
    static char in[4096];
    ssize_t length = read(conn->sockfd, in, sizeof(in));

    if (length > 0) {
        char *data = malloc(sizeof(char) * length);
        memcpy(data, in, length);
        data[length - 1] = '\0';

        return data;
    } else {
        return NULL;
    }
}

void tm_conn_write(struct tm_conn *conn, const char *data)
{
    write(conn->sockfd, data, strlen(data));
}
