#define PURPLE_PLUGINS

#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <gdk/gdk.h>

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

#define INITIAL_CHAT_ID 1000

static const char *USER_ID_SYSTEM = "system";

static const char *ICON = "irc";

static const char *PREF_HOST = "host";
static const char *PREF_HOST_DEFAULT = "localhost";

static const char *PREF_PORT = "port";
static const int PREF_PORT_DEFAULT = 4080;

static const char *PREF_SESSION = "session";

static const char *ROOM_FIELD_ID = "id";
static const char *ROOM_FIELD_NAME = "name";
static const char *ROOM_FIELD_TYPE = "type";
static const char *ROOM_FIELD_DATA = "data";

void tm_on_read(char *data)
{
    tm_log_write(LOG_DEBUG, "> %s", data);
}

void tm_on_write(char *data)
{
    tm_log_write(LOG_DEBUG, "< %s", data);
}

PurpleConversation *get_conversation_for_room(const char *room_id)
{
    PurpleConversation *conv = NULL;

    GList *list = purple_get_conversations();
    while (list != NULL) {
        PurpleConversation *list_conv = list->data;
        char *list_room_id = purple_conversation_get_data(list_conv,
                                                          ROOM_FIELD_ID);

        if (list_room_id) {
            if (strcmp(list_room_id, room_id) == 0) {
                conv = list_conv;
            }
        }

        list = list->next;
    }

    return conv;
}

PurpleConversation *get_conversation_for_chat(int id)
{
    PurpleConversation *conv = NULL;

    GList *list = purple_get_conversations();
    while (list != NULL) {
        PurpleConversation *list_conv = list->data;
        PurpleConvChat *list_conv_chat =
            purple_conversation_get_chat_data(list_conv);

        if (list_conv_chat != NULL) {
            if (list_conv_chat->id == id) {
                conv = list_conv;
            }
        }

        list = list->next;
    }

    return conv;
}

typedef struct
{
    PurpleConversation *conv;
    char *who;
    char *message;
    time_t when;
} write_message_data_t;

gboolean write_message(void *_write_message_data)
{
    write_message_data_t *write_message_data = _write_message_data;

    purple_conversation_write(write_message_data->conv,
                              write_message_data->who,
                              write_message_data->message,
                              PURPLE_MESSAGE_RECV,
                              write_message_data->when);
}

typedef struct {
    tm_client *tm_client;

    PurpleRoomlist *roomlist;
    GMutex roomlist_mutex;
} protocol_data_t;

void tm_on_auth_login(tm_response *response, void *_account)
{
    PurpleAccount *account = _account;
    PurpleConnection *conn = purple_account_get_connection(account);

    if (response->ok) {
        protocol_data_t *protocol_data =
            purple_connection_get_protocol_data(conn);

        tm_client *tm_client = protocol_data->tm_client;

        tm_client_send(tm_client,
                       tm_api_status_req(),
                       NULL,
                       NULL);

        purple_account_set_string(account, PREF_SESSION, response->value);
        purple_connection_set_state(conn, PURPLE_CONNECTED);
    } else {
        purple_connection_set_state(conn, PURPLE_CONNECTING);
    }
}

void tm_on_room_self(const char *room_id,
                     const char *room_name,
                     const char *room_type,
                     const char *room_data,
                     void *_protocol_data)
{
    protocol_data_t *protocol_data = _protocol_data;

    g_mutex_lock(&protocol_data->roomlist_mutex);

    PurpleRoomlist *roomlist = protocol_data->roomlist;

    PurpleRoomlistRoom *room =
        purple_roomlist_room_new(PURPLE_ROOMLIST_ROOMTYPE_ROOM,
                                 g_strdup(room_name),
                                 NULL);

    purple_roomlist_room_add_field(roomlist, room, room_id);
    purple_roomlist_room_add_field(roomlist, room, room_type);
    purple_roomlist_room_add_field(roomlist, room, room_data);

    purple_roomlist_room_add(roomlist, room);

    purple_roomlist_set_in_progress(roomlist, FALSE);

    g_mutex_unlock(&protocol_data->roomlist_mutex);
}

void tm_on_room_any(const char *room_id,
                    const char *action,
                    const char *user_id,
                    const char *user_credential,
                    void *_protocol_data)
{
    protocol_data_t *protocol_data = _protocol_data;

    PurpleConversation *conv = get_conversation_for_room(room_id);
    if (conv != NULL) {
        PurpleConvChat *conv_chat = purple_conversation_get_chat_data(conv);

        GList *users = NULL;
        GList *flags = NULL;

        users = g_list_append(users, (void *) user_credential);
        flags = g_list_append(flags, GINT_TO_POINTER(PURPLE_CBFLAGS_NONE));

        purple_conv_chat_add_users(conv_chat, users, NULL, flags, FALSE);
    }
}

void tm_on_msg_recv(const char *room_id,
                    const char *timestamp,
                    const char *user_id,
                    const char *user_credential,
                    const char *message_data,
                    void *_protocol_data)
{
    protocol_data_t *protocol_data = _protocol_data;

    PurpleConversation *conv = get_conversation_for_room(room_id);
    if (conv != NULL) {
        write_message_data_t *write_message_data =
            malloc(sizeof(write_message_data_t));

        time_t when = atol(timestamp) / 1000;
        if (when == 0) {
            when = time(NULL);
        }

        write_message_data->conv = conv;
        write_message_data->who = g_strdup(user_credential);
        write_message_data->message = g_strdup(message_data);
        write_message_data->when = when;

        gdk_threads_add_idle(&write_message, write_message_data);
    }
}

void tm_on_status_recv(const char *user_id,
                       const char *user_credential,
                       const char *status,
                       void *_protocol_data)
{
    protocol_data_t *protocol_data = _protocol_data;
}

tm_handler **tm_handlers_load(protocol_data_t *protocol_data)
{
    static tm_api_room_self__callback tm_api_room_self__callback;
    tm_api_room_self__callback.handle = &tm_on_room_self;
    tm_api_room_self__callback.data = protocol_data;

    static tm_api_room_any__callback tm_api_room_any__callback;
    tm_api_room_any__callback.handle = &tm_on_room_any;
    tm_api_room_any__callback.data = protocol_data;

    static tm_api_msg_recv__callback tm_api_msg_recv__callback;
    tm_api_msg_recv__callback.handle = &tm_on_msg_recv;
    tm_api_msg_recv__callback.data = protocol_data;

    static tm_api_status_recv__callback tm_api_status_recv__callback;
    tm_api_status_recv__callback.handle = &tm_on_status_recv;
    tm_api_status_recv__callback.data = protocol_data;

    tm_handler **tm_handlers = malloc(sizeof(tm_handler *) * 5);

    tm_handlers[0] = tm_api_room_self(&tm_api_room_self__callback);
    tm_handlers[1] = tm_api_room_any(&tm_api_room_any__callback);
    tm_handlers[2] = tm_api_msg_recv(&tm_api_msg_recv__callback);
    tm_handlers[3] = tm_api_status_recv(&tm_api_status_recv__callback);
    tm_handlers[4] = NULL;

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

static GList *protocol_chat_info(PurpleConnection *conn)
{
    struct proto_chat_entry *entry_id =
        malloc(sizeof(struct proto_chat_entry));
    entry_id->label = ROOM_FIELD_ID;
    entry_id->identifier = ROOM_FIELD_ID;
    entry_id->required = FALSE;
    entry_id->is_int = FALSE;
    entry_id->secret = FALSE;

    struct proto_chat_entry *entry_name =
        malloc(sizeof(struct proto_chat_entry));
    entry_name->label = ROOM_FIELD_NAME;
    entry_name->identifier = ROOM_FIELD_NAME;
    entry_name->required = FALSE;
    entry_name->is_int = FALSE;
    entry_name->secret = FALSE;

    struct proto_chat_entry *entry_type =
        malloc(sizeof(struct proto_chat_entry));
    entry_type->label = ROOM_FIELD_TYPE;
    entry_type->identifier = ROOM_FIELD_TYPE;
    entry_type->required = FALSE;
    entry_type->is_int = FALSE;
    entry_type->secret = FALSE;

    struct proto_chat_entry *entry_data =
        malloc(sizeof(struct proto_chat_entry));
    entry_data->label = ROOM_FIELD_DATA;
    entry_data->identifier = ROOM_FIELD_DATA;
    entry_data->required = FALSE;
    entry_data->is_int = FALSE;
    entry_data->secret = FALSE;

    GList *info = NULL;
    info = g_list_append(info, entry_id);
    info = g_list_append(info, entry_name);
    info = g_list_append(info, entry_type);
    info = g_list_append(info, entry_data);
    return info;
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

        protocol_data_t *protocol_data = malloc(sizeof(protocol_data_t));
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

    protocol_data_t *protocol_data = purple_connection_get_protocol_data(conn);

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

static void protocol_join_chat(PurpleConnection *conn,
                               GHashTable *components)
{
    PurpleAccount *account = purple_connection_get_account(conn);

    char *room_id = g_hash_table_lookup(components, ROOM_FIELD_ID);
    char *room_name = g_hash_table_lookup(components, ROOM_FIELD_NAME);
    char *room_type = g_hash_table_lookup(components, ROOM_FIELD_TYPE);
    char *room_data = g_hash_table_lookup(components, ROOM_FIELD_DATA);

    PurpleChat *chat = purple_blist_find_chat(account, room_id);

    if (chat == NULL) {
        GHashTable *_components = g_hash_table_new_full(g_str_hash,
                                                        g_str_equal,
                                                        g_free,
                                                        g_free);
        GHashTableIter iter;
        gpointer key;
        gpointer value;

        g_hash_table_iter_init(&iter, components);
        while (g_hash_table_iter_next(&iter, &key, &value)) {
            g_hash_table_insert(_components,
                                g_strdup(key),
                                g_strdup(value));
        }

        chat = purple_chat_new(account, g_strdup(room_name), _components);

        purple_blist_add_chat(chat, NULL, NULL);
    }

    PurpleConversation *conv =
        purple_find_conversation_with_account(PURPLE_CONV_TYPE_CHAT,
                                              room_id,
                                              account);

    if (conv == NULL) {
        conv = purple_conversation_new(PURPLE_CONV_TYPE_CHAT,
                                       account,
                                       room_id);

        purple_conversation_set_data(conv, ROOM_FIELD_ID, room_id);
        purple_conversation_set_logging(conv, TRUE);
        purple_conversation_update(conv, PURPLE_CONV_UPDATE_LOGGING);

        conn->buddy_chats = g_slist_append(conn->buddy_chats, conv);

        static int id = INITIAL_CHAT_ID;

        PurpleConvChat *conv_chat = purple_conversation_get_chat_data(conv);
        conv_chat->id = ++id;

        GList *users = NULL;
        GList *flags = NULL;

        users = g_list_append(users, (void *) USER_ID_SYSTEM);
        flags = g_list_append(flags, GINT_TO_POINTER(PURPLE_CBFLAGS_FOUNDER));

        purple_conv_chat_add_users(conv_chat, users, NULL, flags, FALSE);

        protocol_data_t *protocol_data =
            purple_connection_get_protocol_data(conn);

        tm_client *tm_client = protocol_data->tm_client;

        tm_client_send(tm_client,
                       tm_api_room_list_room(room_id),
                       NULL,
                       NULL);

        tm_client_send(tm_client,
                       tm_api_msg_req(room_id),
                       NULL,
                       NULL);
    }

    purple_conversation_present(conv);
}

static int protocol_chat_send(PurpleConnection *conn,
                              int id,
                              const char *message,
                              PurpleMessageFlags flags)
{
    PurpleConversation *conv = get_conversation_for_chat(id);
    if (conv != NULL) {
        char *room_id = purple_conversation_get_data(conv, ROOM_FIELD_ID);

        protocol_data_t *protocol_data =
            purple_connection_get_protocol_data(conn);

        tm_client *tm_client = protocol_data->tm_client;

        tm_client_send(tm_client,
                       tm_api_msg_send(room_id, message),
                       NULL,
                       NULL);
    }
}

static PurpleRoomlist *protocol_roomlist_get_list(PurpleConnection *conn)
{
    PurpleAccount *account = purple_connection_get_account(conn);

    protocol_data_t *protocol_data = purple_connection_get_protocol_data(conn);

    g_mutex_lock(&protocol_data->roomlist_mutex);

    tm_client *tm_client = protocol_data->tm_client;
    PurpleRoomlist *roomlist = protocol_data->roomlist;

    if (roomlist != NULL) {
        purple_roomlist_unref(roomlist);
    }

    roomlist = purple_roomlist_new(account);

    PurpleRoomlistField *field_id =
        purple_roomlist_field_new(PURPLE_ROOMLIST_FIELD_STRING,
                                  ROOM_FIELD_ID,
                                  ROOM_FIELD_ID,
                                  FALSE);
    PurpleRoomlistField *field_type =
        purple_roomlist_field_new(PURPLE_ROOMLIST_FIELD_STRING,
                                  ROOM_FIELD_TYPE,
                                  ROOM_FIELD_TYPE,
                                  FALSE);
    PurpleRoomlistField *field_data =
        purple_roomlist_field_new(PURPLE_ROOMLIST_FIELD_STRING,
                                  ROOM_FIELD_DATA,
                                  ROOM_FIELD_DATA,
                                  FALSE);

    GList *fields = NULL;
    fields = g_list_append(fields, field_id);
    fields = g_list_append(fields, field_type);
    fields = g_list_append(fields, field_data);

    purple_roomlist_set_fields(roomlist, fields);

    protocol_data->roomlist = roomlist;

    g_mutex_unlock(&protocol_data->roomlist_mutex);

    tm_client_send(tm_client,
                   tm_api_room_list(),
                   NULL,
                   NULL);

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
    .chat_info = protocol_chat_info,
    .login = protocol_login,
    .close = protocol_close,
    .join_chat = protocol_join_chat,
    .chat_send = protocol_chat_send,
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
