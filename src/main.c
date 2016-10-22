#include <stdlib.h>
#include <stdio.h>

#include "log.h"
#include "config.h"
#include "client.h"

int main(int argc, const char *argv[]) {

    tm_log_open();

    struct tm_config *config = tm_config_load(CONFIG_PATH);

    struct tm_client *client = tm_client_connect(config);
    if (client != NULL) {

        char* data = tm_client_read(client);
        free(data);

        tm_client_write(client, "auth:login 1234 asdf@yellow 1234");
        data = tm_client_read(client);
        printf("> %s\n", data);
        free(data);

        tm_client_write(client, "sys:exit");

        tm_client_disconnect(client);
    }

    tm_config_free(config);

    tm_log_close();

    return EXIT_SUCCESS;
}
