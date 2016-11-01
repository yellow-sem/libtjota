#include "conn.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "log.h"
#include "config.h"

tm_conn *tm_conn_new()
{
    return malloc(sizeof(tm_conn));
}

void tm_conn_free(tm_conn *conn)
{
    free(conn);
}

tm_conn *tm_conn_open(tm_config *config)
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

    tm_conn *conn = tm_conn_new();
    conn->hostent = hostent;
    conn->sockfd = sockfd;
    return conn;
}

void tm_conn_close(tm_conn *conn)
{
    close(conn->sockfd);
    tm_conn_free(conn);
}

bool tm_conn_select(tm_conn *conn)
{
    fd_set fd_set;
    FD_ZERO(&fd_set);
    FD_SET(conn->sockfd, &fd_set);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 10000;

    return select(conn->sockfd + 1, &fd_set, NULL, NULL, &timeout) > 0;
}

char *tm_conn_read(tm_conn *conn)
{
    static char in[4096];

    int i = 0;
    char c = '\0';
    while (true) {
        read(conn->sockfd, &c, sizeof(char));
        if (c == '\n' || c == '\0') {
            break;
        }
        in[i] = c;
        i++;
    }

    if (i > 0) {
        int length = i;
        char *data = malloc(sizeof(char) * length + 1);
        memcpy(data, in, length);
        data[length] = '\0';
        return data;
    } else {
        return NULL;
    }
}

void tm_conn_write(tm_conn *conn, const char *data)
{
    char *out = malloc(sizeof(char) * (strlen(data) + 2));
    sprintf(out, "%s\n", data);

    write(conn->sockfd, out, strlen(out));

    free(out);
}
