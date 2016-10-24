#define PURPLE_PLUGINS

#include <glib.h>
#include <stdio.h>

#include "notify.h"
#include "plugin.h"
#include "version.h"
#include "accountopt.h"

#include "libtjota/log.h"
#include "libtjota/config.h"
#include "libtjota/conn.h"
#include "libtjota/client.h"
#include "libtjota/api.h"

static const char *ICON = "irc";

static const char *PREF_HOST = "host";
static const char *PREF_HOST_DEFAULT = "localhost";

static const char *PREF_PORT = "port";
static const int PREF_PORT_DEFAULT = 4080;

void tm_on_read(char *data)
{
    printf("> %s\n", data);
}

void tm_on_write(char *data)
{
    printf("< %s\n", data);
}

static tm_client *tm_client_s = NULL;

void tm_on_sys_exit(tm_response *response, void *data)
{
    if (tm_client_s != NULL) {

        tm_client_stop(tm_client_s);
        tm_conn_close(tm_client_s->conn);
        tm_client_free(tm_client_s);

        tm_client_s = NULL;
    }
}

void tm_on_auth_login(tm_response *response, void *_conn)
{
    PurpleConnection *conn = _conn;

    if (response->ok) {
        purple_connection_set_state(conn, PURPLE_CONNECTED);
    } else {
        purple_connection_set_state(conn, PURPLE_CONNECTING);
    }
}

static gboolean plugin_load(PurplePlugin *plugin)
{
    tm_log_open();
    return TRUE;
}

static gboolean plugin_unload(PurplePlugin *plugin)
{
    if (tm_client_s != NULL) {

        tm_client_stop(tm_client_s);
        tm_conn_close(tm_client_s->conn);
        tm_client_free(tm_client_s);

        tm_client_s = NULL;
    }

    tm_log_close();
}

static const char *protocol_list_icon(PurpleAccount *account,
                                      PurpleBuddy *buddy)
{
    return ICON;
}

static GList *protocol_status_types(PurpleAccount *account)
{
    GList *type_list = NULL;

    PurpleStatusType *type_online =
        purple_status_type_new(PURPLE_STATUS_AVAILABLE,
                               NULL, NULL, FALSE);

    PurpleStatusType *type_offline =
        purple_status_type_new(PURPLE_STATUS_OFFLINE,
                               NULL, NULL, FALSE);

    type_list = g_list_append(type_list, type_online);
    type_list = g_list_append(type_list, type_offline);

    return type_list;
}

static void protocol_login(PurpleAccount *account)
{
    PurpleConnection *conn = purple_account_get_connection(account);

    tm_config *tm_config = tm_config_new();
    tm_config->host = (char*) purple_account_get_string(account,
                                                        PREF_HOST,
                                                        PREF_HOST_DEFAULT);
    tm_config->port = purple_account_get_int(account,
                                             PREF_PORT,
                                             PREF_PORT_DEFAULT);

    tm_conn *tm_conn = tm_conn_open(tm_config);

    if (tm_conn != NULL) {
        purple_connection_set_state(conn, PURPLE_CONNECTING);

        static tm_handler *tm_handlers[] = { NULL };

        tm_client_s = tm_client_new(tm_conn,
                                  tm_handlers,
                                  &tm_on_read,
                                  &tm_on_write);
        tm_client_start(tm_client_s);

        const char* username = purple_account_get_username(account);
        const char* password = purple_account_get_password(account);

        tm_client_send(tm_client_s,
                       tm_api_auth_login_credential(username, password),
                       &tm_on_auth_login,
                       conn);
    }

    tm_config_free(tm_config);
}

static void protocol_close(PurpleConnection *conn)
{
    purple_connection_set_state(conn, PURPLE_DISCONNECTED);

    if (tm_client_s != NULL) {
        tm_client_send(tm_client_s,
                       tm_api_sys_exit(),
                       *tm_on_sys_exit,
                       NULL);
    }
}

static PurplePluginProtocolInfo protocol_info = {
    .options = OPT_PROTO_CHAT_TOPIC,
    .list_icon = protocol_list_icon,
    .status_types = protocol_status_types,
    .login = protocol_login,
    .close = protocol_close,
    .struct_size = sizeof(PurplePluginProtocolInfo),
};

static PurplePluginInfo plugin_info = {
    .magic = PURPLE_PLUGIN_MAGIC,
    .major_version = PURPLE_MAJOR_VERSION,
    .minor_version = PURPLE_MINOR_VERSION,
    .type = PURPLE_PLUGIN_PROTOCOL,
    .ui_requirement = NULL,
    .flags = 0,
    .dependencies = NULL,
    .priority = PURPLE_PRIORITY_DEFAULT,

    .id = "core-yellow-tjota",
    .name = "Tjöta",
    .version = "0.1",
    .summary = "Tjöta Chat Client",
    .description = "Connect to a Tjöta chat server.",
    .author = "Sebastian Nogara <snogaraleal@gmail.com>",
    .homepage = "http://tjota.online",
    
    .load = plugin_load,
    .unload = plugin_unload,
    .destroy = NULL,

    .ui_info = NULL,
    .extra_info = &protocol_info,
    .prefs_info = NULL,
    .actions = NULL,
};

static void plugin_init(PurplePlugin *plugin)
{
    PurpleAccountOption *opt_host =
        purple_account_option_string_new(PREF_HOST,
                                         PREF_HOST,
                                         PREF_HOST_DEFAULT);

    PurpleAccountOption *opt_port =
        purple_account_option_int_new(PREF_PORT,
                                      PREF_PORT,
                                      PREF_PORT_DEFAULT);

    protocol_info.protocol_options =
        g_list_append(protocol_info.protocol_options, opt_host);

    protocol_info.protocol_options =
        g_list_append(protocol_info.protocol_options, opt_port);
}

PURPLE_INIT_PLUGIN(tjota, plugin_init, plugin_info)
