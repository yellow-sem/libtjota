#define PURPLE_PLUGINS

#include <glib.h>
#include <stdio.h>

#include "notify.h"
#include "plugin.h"
#include "version.h"
#include "accountopt.h"
#include "roomlist.h"

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

static const char *PREF_SESSION = "session";

static const char *ROOMLIST_FIELD_ID = "id";
static const char *ROOMLIST_FIELD_TYPE = "type";

void tm_on_read(char *data)
{
    tm_log_write(LOG_DEBUG, "> %s", data);
}

void tm_on_write(char *data)
{
    tm_log_write(LOG_DEBUG, "< %s", data);
}

typedef struct {
    tm_client *tm_client;

    PurpleRoomlist *roomlist;
    GMutex roomlist_mutex;
} protocol_data;

void tm_on_auth_login(tm_response *response, void *_account)
{
    PurpleAccount *account = _account;
    PurpleConnection *conn = purple_account_get_connection(account);

    if (response->ok) {
        purple_account_set_string(account, PREF_SESSION, response->value);
        purple_connection_set_state(conn, PURPLE_CONNECTED);
    } else {
        purple_connection_set_state(conn, PURPLE_CONNECTING);
    }
}

void tm_on_room_self(const char *room_id,
                     const char *room_name,
                     const char *room_type,
                     void *_protocol_data)
{
    protocol_data *protocol_data = _protocol_data;

    g_mutex_lock(&protocol_data->roomlist_mutex);

    PurpleRoomlist *roomlist = protocol_data->roomlist;

    PurpleRoomlistRoom *room =
        purple_roomlist_room_new(PURPLE_ROOMLIST_ROOMTYPE_ROOM,
                                 g_strdup(room_name),
                                 NULL);

    purple_roomlist_room_add_field(roomlist, room, room_id);
    purple_roomlist_room_add_field(roomlist, room, room_type);

    purple_roomlist_room_add(roomlist, room);

    purple_roomlist_set_in_progress(roomlist, FALSE);

    g_mutex_unlock(&protocol_data->roomlist_mutex);
}

tm_handler **tm_handlers_load(protocol_data *protocol_data)
{
    static tm_api_room_self__callback tm_api_room_self__callback;
    tm_api_room_self__callback.handle = &tm_on_room_self;
    tm_api_room_self__callback.data = protocol_data;

    tm_handler **tm_handlers = malloc(sizeof(tm_handler *) * 2);

    tm_handlers[0] = tm_api_room_self(&tm_api_room_self__callback);
    tm_handlers[1] = NULL;

    return tm_handlers;
}

void tm_handlers_unload(tm_handler **tm_handlers)
{
    tm_handler_free_all(tm_handlers);
    free(tm_handlers);
}

static gboolean plugin_load(PurplePlugin *plugin)
{
    tm_log_open();
    return TRUE;
}

static gboolean plugin_unload(PurplePlugin *plugin)
{
    tm_log_close();
    return TRUE;
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
    tm_config->host = (char *) purple_account_get_string(account,
                                                         PREF_HOST,
                                                         PREF_HOST_DEFAULT);
    tm_config->port = purple_account_get_int(account,
                                             PREF_PORT,
                                             PREF_PORT_DEFAULT);

    tm_conn *tm_conn = tm_conn_open(tm_config);

    if (tm_conn != NULL) {
        purple_connection_set_state(conn, PURPLE_CONNECTING);

        protocol_data *protocol_data = malloc(sizeof(protocol_data));

        protocol_data->tm_client = NULL;

        protocol_data->roomlist = NULL;
        g_mutex_init(&protocol_data->roomlist_mutex);

        tm_client *tm_client = tm_client_new(tm_conn,
                                             tm_handlers_load(protocol_data),
                                             &tm_on_read,
                                             &tm_on_write);
        tm_client_start(tm_client);

        protocol_data->tm_client = tm_client;
        purple_connection_set_protocol_data(conn, protocol_data);

        const char *session = purple_account_get_string(account,
                                                        PREF_SESSION,
                                                        NULL);
        if (session == NULL) {
            const char *username = purple_account_get_username(account);
            const char *password = purple_account_get_password(account);

            tm_client_send(tm_client,
                           tm_api_auth_login_credential(username, password),
                           &tm_on_auth_login,
                           account);
        } else {
            tm_client_send(tm_client,
                           tm_api_auth_login_session(session),
                           &tm_on_auth_login,
                           account);
        }
    }

    tm_config_free(tm_config);
}

static void protocol_close(PurpleConnection *conn)
{
    purple_connection_set_state(conn, PURPLE_DISCONNECTED);

    protocol_data *protocol_data = purple_connection_get_protocol_data(conn);

    if (protocol_data != NULL) {
        tm_client *tm_client = protocol_data->tm_client;

        tm_client_send(tm_client,
                       tm_api_sys_exit(),
                       NULL,
                       NULL);

        tm_client_stop(tm_client);
        tm_conn_close(tm_client->conn);
        tm_handlers_unload(tm_client->handlers);
        tm_client_free(tm_client);

        free(protocol_data);
        purple_connection_set_protocol_data(conn, NULL);
    }
}

static PurpleRoomlist *protocol_roomlist_get_list(PurpleConnection *conn)
{
    PurpleAccount *account = purple_connection_get_account(conn);

    protocol_data *protocol_data = purple_connection_get_protocol_data(conn);

    g_mutex_lock(&protocol_data->roomlist_mutex);

    tm_client *tm_client = protocol_data->tm_client;
    PurpleRoomlist *roomlist = protocol_data->roomlist;

    if (roomlist != NULL) {
        purple_roomlist_unref(roomlist);
    }

    roomlist = purple_roomlist_new(account);

    PurpleRoomlistField *field_id =
        purple_roomlist_field_new(PURPLE_ROOMLIST_FIELD_STRING,
                                  ROOMLIST_FIELD_ID,
                                  ROOMLIST_FIELD_ID,
                                  FALSE);
    PurpleRoomlistField *field_type =
        purple_roomlist_field_new(PURPLE_ROOMLIST_FIELD_STRING,
                                  ROOMLIST_FIELD_TYPE,
                                  ROOMLIST_FIELD_TYPE,
                                  FALSE);

    GList *fields = NULL;
    fields = g_list_append(fields, field_id);
    fields = g_list_append(fields, field_type);

    purple_roomlist_set_fields(roomlist, fields);

    protocol_data->roomlist = roomlist;

    tm_client_send(tm_client,
                   tm_api_room_list(),
                   NULL,
                   NULL);

    g_mutex_unlock(&protocol_data->roomlist_mutex);

    return roomlist;
}

void protocol_roomlist_cancel(PurpleRoomlist *roomlist)
{
    purple_roomlist_set_in_progress(roomlist, FALSE);
    purple_roomlist_unref(roomlist);
}

static PurplePluginProtocolInfo protocol_info = {
    .options = OPT_PROTO_CHAT_TOPIC,
    .list_icon = protocol_list_icon,
    .status_types = protocol_status_types,
    .login = protocol_login,
    .close = protocol_close,
    .roomlist_get_list = protocol_roomlist_get_list,
    .roomlist_cancel = protocol_roomlist_cancel,
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
