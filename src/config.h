#ifndef CONFIG_H
#define CONFIG_H

#include <glib.h>
#include <stdbool.h>

static const char *CONFIG_PATH = "config.ini";

static const char *CONFIG_G_SERVER = "server";
static const char *CONFIG_G_SERVER_K_HOST = "host";
static const char *CONFIG_G_SERVER_K_PORT = "port";

struct tm_config
{
    char *host;
    int port;
};

bool tm_config_check_group(GKeyFile *file,
                           const char *path,
                           const char *group);
bool tm_config_check_key(GKeyFile *file,
                         const char *path,
                         const char *group,
                         const char *key);

struct tm_config *tm_config_new();
struct tm_config *tm_config_free(struct tm_config *config);
struct tm_config *tm_config_load(const char *path);

#endif
