#ifndef CLIENT_H
#define CLIENT_H

#include <stdbool.h>
#include <regex.h>
#include <glib.h>

#include "conn.h"

struct tm_handler
{
    const char *regex_s;
    regex_t *regex_c;
    void (*handle)(int argc, char **argv);
};

struct tm_handler *tm_handler_new();
void tm_handler_free(struct tm_handler *handler);

struct tm_request
{
    char *command;
    char *ident;
    int argc;
    char **argv;
};

struct tm_response
{
    char *command;
    char *ident;
    char *value;
    bool ok;
};

struct tm_client
{
    struct tm_conn *conn;
    struct tm_handler **handlers;

    void (*on_read)(char *data);
    void (*on_write)(char *data);

    GAsyncQueue *queue;
    GHashTable *table;

    bool run;
    pthread_t thread_outgoing;
    pthread_t thread_incoming;
};

struct tm_client *tm_client_new(struct tm_conn *conn,
                                struct tm_handler **handlers,
                                void (*on_read)(char *data),
                                void (*on_write)(char *data));
void tm_client_free(struct tm_client *client);

void *tm_client_thread_routine_outgoing(void *data);
void *tm_client_thread_routine_incoming(void *data);

void tm_client_start(struct tm_client *client);
void tm_client_stop(struct tm_client *client);

void tm_client_send(struct tm_client *client,
                    struct tm_request *request);

struct tm_response *tm_client_poll(struct tm_client *client,
                                   struct tm_request *request);

struct tm_response *tm_client_wait(struct tm_client *client,
                                   struct tm_request *request);

#endif
