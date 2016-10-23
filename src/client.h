#ifndef CLIENT_H
#define CLIENT_H

#include <stdbool.h>
#include <glib.h>

#include "conn.h"
#include "handler.h"
#include "request.h"
#include "response.h"

struct tm_client
{
    struct tm_conn *conn;
    struct tm_handler **handlers;

    void (*on_read)(char *data);
    void (*on_write)(char *data);

    GAsyncQueue *queue;
    GMutex mutex;
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

void *tm_client_thread_routine_outgoing(void *_client);
void *tm_client_thread_routine_incoming(void *_client);

void tm_client_start(struct tm_client *client);
void tm_client_stop(struct tm_client *client);

void tm_client_send(struct tm_client *client,
                    struct tm_request *request);

struct tm_response *tm_client_poll(struct tm_client *client,
                                   struct tm_request *request);

struct tm_response *tm_client_wait(struct tm_client *client,
                                   struct tm_request *request);

#endif
