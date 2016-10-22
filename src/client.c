#include "client.h"

#include <stdlib.h>
#include <glib.h>
#include <stdio.h>
#include <string.h>

#include "regex.h"
#include "string.h"

struct tm_handler *tm_handler_new()
{
    struct tm_handler *handler = malloc(sizeof(struct tm_handler));
    handler->regex_s = NULL;
    handler->regex_c = NULL;
    handler->handle = NULL;
    return handler;
}

void tm_handler_free(struct tm_handler *handler)
{
    if (handler->regex_c != NULL) tm_regex_free(handler->regex_c);
    free(handler);
}

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
    client->table = g_hash_table_new(g_str_hash, g_str_equal);

    return client;
}

void tm_client_free(struct tm_client *client)
{
    g_async_queue_unref(client->queue);
    g_hash_table_unref(client->table);
    free(client);
}

void *tm_client_thread_routine_outgoing(void *data)
{
    struct tm_client *client = data;

    do {
        struct tm_request *request = g_async_queue_try_pop(client->queue);
        if (request == NULL) {
            continue;
        }

        int argc = 2 + request->argc;
        char **argv = malloc(sizeof(char *) * argc);

        argv[0] = request->command;
        argv[1] = request->ident;

        int i;

        for (i = 0; i < request->argc; i++) {
            char *arg = request->argv[i];

            char *argq = malloc(2 + strlen(arg) + 1);
            sprintf(argq, "'%s'", arg);

            argv[2 + i] = argq;
        }

        char *data = tm_string_join(" ", argc, argv);

        tm_conn_write(client->conn, data);
        client->on_write(data);

        free(data);

        for (i = 0; i < request->argc; i++) {
            free(argv[2 + i]);
        }

        free(argv);

    } while (client->run);
}

void *tm_client_thread_routine_incoming(void *data)
{
    struct tm_client *client = data;

    do {
        printf("in\n");
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
