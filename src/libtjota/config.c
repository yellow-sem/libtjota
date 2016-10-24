#include "config.h"

#include <stdlib.h>

#include "log.h"

bool tm_config_check_group(GKeyFile *file,
                           const char *path,
                           const char *group)
{

    if (g_key_file_has_group(file, group)) {
        return true;
    } else {
        tm_log_write(LOG_ERR,
                     "Group '%s' missing from '%s'",
                     CONFIG_G_SERVER,
                     path);
        return false;
    }
}

bool tm_config_check_key(GKeyFile *file,
                         const char *path,
                         const char *group,
                         const char *key)
{
    GError *error = NULL;

    if (g_key_file_has_key(file, group, key, &error)) {
        return true;
    } else {
        tm_log_write(LOG_ERR, "'%s' missing from group '%s' in '%s'",
                     key, group, path);
    }

    if (error != NULL) {
        tm_log_gerror(error);
        g_error_free(error);
    }

    return false;
}

tm_config *tm_config_new()
{
    return malloc(sizeof(tm_config));
}

tm_config *tm_config_free(tm_config *config)
{
    free(config);
}

tm_config *tm_config_load(const char *path)
{
    GKeyFile *file = g_key_file_new();
    GError *error = NULL;

    if (g_key_file_load_from_file(file, path, G_KEY_FILE_NONE, &error)) {
        tm_log_write(LOG_INFO,
                     "Config file '%s' loaded",
                     path);
    } else {
        tm_log_write(LOG_ERR,
                     "Error loading '%s': %s",
                     path,
                     error->message);
    }

    if (error != NULL) {
        tm_log_gerror(error);
        g_error_free(error);

        return NULL;
    }

    tm_config *config = tm_config_new();

    bool success = true;

    if (tm_config_check_group(file, path,
                              CONFIG_G_SERVER)) {

        if (tm_config_check_key(file, path,
                                CONFIG_G_SERVER,
                                CONFIG_G_SERVER_K_HOST)) {

            error = NULL;
            config->host = g_key_file_get_string(file,
                                                 CONFIG_G_SERVER,
                                                 CONFIG_G_SERVER_K_HOST,
                                                 &error);
            if (error != NULL) {
                tm_log_gerror(error);
                g_error_free(error);
                success = false;
            }
        } else {
            success = false;
        }

        if (tm_config_check_key(file, path,
                                CONFIG_G_SERVER,
                                CONFIG_G_SERVER_K_PORT)) {

            error = NULL;
            config->port = g_key_file_get_integer(file,
                                                  CONFIG_G_SERVER,
                                                  CONFIG_G_SERVER_K_PORT,
                                                  &error);
            if (error != NULL) {
                tm_log_gerror(error);
                g_error_free(error);
                success = false;
            }
        } else {
            success = false;
        }
    }

    g_key_file_free(file);

    if (success) {
        return config;
    } else {
        tm_config_free(config);
        return NULL;
    }
}
