#ifndef CONFIG_H
#define CONFIG_H

#include <glib.h>
#include <stdbool.h>

static const char *CONFIG_PATH = "config.ini";

static const char *CONFIG_G_SERVER = "server";
static const char *CONFIG_G_SERVER_K_HOST = "host";
static const char *CONFIG_G_SERVER_K_PORT = "port";

typedef struct
{
    char *host;
    int port;
} tm_config;

bool tm_config_check_group(GKeyFile *file,
                           const char *path,
                           const char *group);
bool tm_config_check_key(GKeyFile *file,
                         const char *path,
                         const char *group,
                         const char *key);

tm_config *tm_config_new();
tm_config *tm_config_free(tm_config *config);
tm_config *tm_config_load(const char *path);

#endif
