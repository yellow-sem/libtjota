#include <stdlib.h>
#include <stdio.h>

#include "log.h"
#include "config.h"
#include "conn.h"
#include "client.h"
#include "api.h"

void on_read(char *data) {
}

void on_write(char *data) {
}

int main(int argc, const char *argv[]) {

    tm_log_open();

    struct tm_config *config = tm_config_load(CONFIG_PATH);

    struct tm_conn *conn = tm_conn_open(config);
    if (conn != NULL) {

        struct tm_handler **handlers = { NULL };
        struct tm_client *client = tm_client_new(conn,
                                                 handlers,
                                                 &on_read,
                                                 &on_write);

        tm_client_start(client);

        struct tm_request request;
        request.command = "sys:exit";
        request.ident = "1234";
        request.argc = 0;
        request.argv = NULL;

        tm_client_send(client, &request);

        tm_client_stop(client);

        tm_client_free(client);

        tm_conn_close(conn);
    }

    tm_config_free(config);

    tm_log_close();

    return EXIT_SUCCESS;
}
