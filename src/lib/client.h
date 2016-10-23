#ifndef CLIENT_H
#define CLIENT_H

#include <stdbool.h>
#include <glib.h>

#include "conn.h"
#include "handler.h"
#include "request.h"
#include "response.h"

typedef struct
{
    tm_conn *conn;
    tm_handler **handlers;

    void (*on_read)(char *data);
    void (*on_write)(char *data);

    GAsyncQueue *queue;
    GMutex mutex;
    GHashTable *table;

    bool run;
    pthread_t thread_outgoing;
    pthread_t thread_incoming;
} tm_client;

tm_client *tm_client_new(tm_conn *conn,
                         tm_handler **handlers,
                         void (*on_read)(char *data),
                         void (*on_write)(char *data));
void tm_client_free(tm_client *client);

void *tm_client_thread_routine_outgoing(void *_client);
void *tm_client_thread_routine_incoming(void *_client);

void tm_client_start(tm_client *client);
void tm_client_stop(tm_client *client);

void tm_client_send(tm_client *client,
                    tm_request *request);

tm_response *tm_client_poll(tm_client *client,
                            tm_request *request);

tm_response *tm_client_wait(tm_client *client,
                            tm_request *request);

#endif
