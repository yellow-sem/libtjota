#include "client.h"

#include <stdlib.h>
#include <glib.h>

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
    client->table = g_hash_table_new(g_str_hash, g_str_equal);

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
                g_mutex_lock(&client->mutex);
                g_hash_table_insert(client->table, response->ident, response);
                g_mutex_unlock(&client->mutex);
            }
        }

        client->on_read(data);
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
                    tm_request *request)
{
    g_async_queue_push(client->queue, request);
}

tm_response *tm_client_poll(tm_client *client,
                            tm_request *request)
{
    tm_response *response;

    g_mutex_lock(&client->mutex);
    response = g_hash_table_lookup(client->table, request->ident);
    if (response != NULL) {
        g_hash_table_remove(client->table, request->ident);
    }
    g_mutex_unlock(&client->mutex);

    return response;
}

tm_response *tm_client_wait(tm_client *client,
                            tm_request *request)
{
    tm_response *response = NULL;

    do {
        response = tm_client_poll(client, request);
    } while (response == NULL);

    return response;
}
