#include "client.h"

#include <stdlib.h>
#include <string.h>
#include <glib.h>

tm_callback *tm_callback_new(void (*func)(tm_response *response, void *data),
                             void *data)
{
    tm_callback *callback = malloc(sizeof(tm_callback));
    callback->func = func;
    callback->data = data;
    return callback;
}

void tm_callback_free(tm_callback *callback)
{
    free(callback);
}

tm_client *tm_client_new(tm_conn *conn,
                         tm_handler **handlers,
                         void (*on_read)(char *data),
                         void (*on_write)(char *data))
{
    tm_client *client = malloc(sizeof(tm_client));

    client->conn = conn;
    client->handlers = handlers;

    client->on_read = on_read;
    client->on_write = on_write;

    client->queue = g_async_queue_new();
    g_mutex_init(&client->mutex);
    client->table = g_hash_table_new_full(g_str_hash, g_str_equal,
                                          &free, NULL);

    return client;
}

void tm_client_free(tm_client *client)
{
    g_async_queue_unref(client->queue);
    g_hash_table_unref(client->table);
    free(client);
}

void *tm_client_thread_routine_outgoing(void *_client)
{
    tm_client *client = _client;

    do {
        tm_request *request = g_async_queue_try_pop(client->queue);
        if (request == NULL) {
            continue;
        }

        char *data = tm_request_encode(request);
        tm_conn_write(client->conn, data);
        client->on_write(data);
        free(data);

        tm_request_free(request);

    } while (client->run);
}

void *tm_client_thread_routine_incoming(void *_client)
{
    tm_client *client = _client;

    do {
        bool select = tm_conn_select(client->conn);
        if (!select) {
            continue;
        }

        char *data = tm_conn_read(client->conn);
        if (data == NULL) {
            continue;
        }

        client->on_read(data);

        bool match = false;

        int i = 0;
        tm_handler *handler = client->handlers[i];
        while (handler != NULL) {
            match |= tm_handler_handle(handler, data);
            handler = client->handlers[++i];
        }

        if (!match) {
            tm_response *response = tm_response_decode(data);

            if (response != NULL) {
                tm_callback *callback = NULL;

                g_mutex_lock(&client->mutex);
                callback = g_hash_table_lookup(client->table, response->ident);
                if (callback != NULL) {
                    g_hash_table_remove(client->table, response->ident);
                }
                g_mutex_unlock(&client->mutex);

                if (callback != NULL) {
                    callback->func(response, callback->data);
                    tm_callback_free(callback);
                }

                tm_response_free(response);
            }
        }

        free(data);

    } while (client->run);
}

void tm_client_start(tm_client *client)
{
    client->run = true;

    pthread_create(&client->thread_outgoing,
                   NULL,
                   &tm_client_thread_routine_outgoing,
                   client);

    pthread_create(&client->thread_incoming,
                   NULL,
                   &tm_client_thread_routine_incoming,
                   client);
}

void tm_client_stop(tm_client *client)
{
    client->run = false;

    pthread_join(client->thread_outgoing, NULL);
    pthread_join(client->thread_incoming, NULL);
}

void tm_client_send(tm_client *client,
                    tm_request *request,
                    void (*callback)(tm_response *response, void *data),
                    void *data)
{
    char *key = malloc(strlen(request->ident) + 1);
    strcpy(key, request->ident);

    tm_callback *value = tm_callback_new(callback, data);

    g_mutex_lock(&client->mutex);
    g_hash_table_insert(client->table, key, value);
    g_mutex_unlock(&client->mutex);

    g_async_queue_push(client->queue, request);
}
