#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "libtjota/log.h"
#include "libtjota/config.h"
#include "libtjota/conn.h"
#include "libtjota/client.h"
#include "libtjota/api.h"

static const char *TEST_SESSION_ID = "097c2dcf-791c-4757-b650-23820e12aa6d";

void tm_on_read(char *data)
{
    printf("> %s\n", data);
}

void tm_on_write(char *data)
{
    printf("< %s\n", data);
}

void tm_on_auth_login(tm_response *response, void *data)
{
    printf("received login result: \n");
    if (response->ok) {
        printf("success\n");
    } else {
        printf("error\n");
    }
}

void tm_on_sys_exit(tm_response *response, void *data)
{
    printf("received exit result: \n");
    if (response->ok) {
        printf("success\n");
    } else {
        printf("error\n");
    }
}

void tm_on_room_self(const char *room_id,
                     const char *room_name,
                     const char *room_type,
                     const char *room_data,
                     void *data)
{
    printf("joined room %s name='%s' type=%s\n",
           room_id, room_name, room_type);
}

int main(int argc, const char *argv[])
{

    tm_log_open();

    tm_config *config = tm_config_load(CONFIG_PATH);
    if (config == NULL) {
        return EXIT_FAILURE;
    }

    tm_conn *conn = tm_conn_open(config);
    if (conn != NULL) {

        tm_api_room_self__callback tm_api_room_self__callback;
        tm_api_room_self__callback.handle = &tm_on_room_self;
        tm_api_room_self__callback.data = NULL;

        tm_handler *handlers[] = {
            tm_api_room_self(&tm_api_room_self__callback),
            NULL
        };
        tm_client *client = tm_client_new(conn,
                                          handlers,
                                          &tm_on_read,
                                          &tm_on_write);

        tm_client_start(client);

        tm_client_send(client,
                       tm_api_auth_login_session(TEST_SESSION_ID),
                       &tm_on_auth_login,
                       NULL);
        sleep(2);

        tm_client_send(client,
                       tm_api_room_list(),
                       NULL,
                       NULL);
        sleep(2);

        tm_client_send(client,
                       tm_api_sys_exit(),
                       &tm_on_sys_exit,
                       NULL);
        sleep(2);

        tm_client_stop(client);
        tm_client_free(client);
        tm_handler_free_all(handlers);
        tm_conn_close(conn);
    }

    free(config->host);
    tm_config_free(config);

    tm_log_close();

    return EXIT_SUCCESS;
}
