#include "client.h"

#include <stdlib.h>
#include <glib.h>

struct tm_client *tm_client_new(struct tm_conn *conn,
                                struct tm_handler **handlers,
                                void (*on_read)(char *data),
                                void (*on_write)(char *data))
{
    struct tm_client *client = malloc(sizeof(struct tm_client));

    client->conn = conn;
    client->handlers = handlers;

    client->on_read = on_read;
    client->on_write = on_write;

    client->queue = g_async_queue_new();
    g_mutex_init(&client->mutex);
    client->table = g_hash_table_new(g_str_hash, g_str_equal);

    return client;
}

void tm_client_free(struct tm_client *client)
{
    g_async_queue_unref(client->queue);
    g_hash_table_unref(client->table);
    free(client);
}

void *tm_client_thread_routine_outgoing(void *_client)
{
    struct tm_client *client = _client;

    do {
        struct tm_request *request = g_async_queue_try_pop(client->queue);
        if (request == NULL) {
            continue;
        }

        char *data = tm_request_encode(request);
        tm_conn_write(client->conn, data);
        client->on_write(data);
        free(data);

    } while (client->run);
}

void *tm_client_thread_routine_incoming(void *_client)
{
    struct tm_client *client = _client;

    do {
        bool select = tm_conn_select(client->conn);
        if (!select) {
            continue;
        }

        char *data = tm_conn_read(client->conn);
        if (data == NULL) {
            continue;
        }

        bool match = false;

        int i = 0;
        struct tm_handler *handler = client->handlers[i];
        while (handler != NULL) {
            match |= tm_handler_handle(handler, data);
            handler = client->handlers[++i];
        }

        if (!match) {
            struct tm_response *response = tm_response_decode(data);
            if (response != NULL) {
                g_mutex_lock(&client->mutex);
                g_hash_table_insert(client->table, response->ident, response);
                g_mutex_unlock(&client->mutex);
            }
        }

        client->on_read(data);
        free(data);

    } while (client->run);
}

void tm_client_start(struct tm_client *client)
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

void tm_client_stop(struct tm_client *client)
{
    client->run = false;

    pthread_join(client->thread_outgoing, NULL);
    pthread_join(client->thread_incoming, NULL);
}

void tm_client_send(struct tm_client *client,
                    struct tm_request *request)
{
    g_async_queue_push(client->queue, request);
}

struct tm_response *tm_client_poll(struct tm_client *client,
                                   struct tm_request *request)
{
    struct tm_response *response;

    g_mutex_lock(&client->mutex);
    response = g_hash_table_lookup(client->table, request->ident);
    if (response != NULL) {
        g_hash_table_remove(client->table, request->ident);
    }
    g_mutex_unlock(&client->mutex);

    return response;
}

struct tm_response *tm_client_wait(struct tm_client *client,
                                   struct tm_request *request)
{
    struct tm_response *response = NULL;

    do {
        response = tm_client_poll(client, request);
    } while (response == NULL);

    return response;
}
