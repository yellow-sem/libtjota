#include <stdlib.h>
#include <stdio.h>

#include "log.h"
#include "config.h"
#include "conn.h"
#include "client.h"
#include "api.h"

void on_read(char *data) {
    printf("> %s\n", data);
}

void on_write(char *data) {
    printf("< %s\n", data);
}

int main(int argc, const char *argv[]) {

    tm_log_open();

    struct tm_config *config = tm_config_load(CONFIG_PATH);

    struct tm_conn *conn = tm_conn_open(config);
    if (conn != NULL) {

        struct tm_handler *handlers[] = { NULL };
        struct tm_client *client = tm_client_new(conn,
                                                 handlers,
                                                 &on_read,
                                                 &on_write);

        tm_client_start(client);

        struct tm_request *request;
        struct tm_response *response;

        request = tm_api_auth_login_credential("asdf@yellow", "445");
        tm_client_send(client, request);
        response = tm_client_wait(client, request);
        tm_request_free(request);
        if (response->ok) {
            printf("success\n");
        } else {
            printf("error\n");
        }
        tm_response_free(response);

        printf("exiting\n");

        request = tm_api_sys_exit();
        tm_client_send(client, request);
        response = tm_client_wait(client, request);
        tm_request_free(request);
        tm_response_free(response);

        tm_client_stop(client);

        tm_client_free(client);

        tm_conn_close(conn);
    }

    tm_config_free(config);

    tm_log_close();

    return EXIT_SUCCESS;
}
