#ifndef API_H
#define API_H

#include "request.h"
#include "handler.h"

tm_request *tm_api_sys_exit();

tm_request *tm_api_auth_login_credential(const char *credential,
                                         const char *password);
tm_request *tm_api_auth_login_session(const char *session);
tm_request *tm_api_auth_logout(const char *session);
tm_request *tm_api_auth_check(const char *credential);

tm_request *tm_api_room_list();
tm_request *tm_api_room_list_room(const char *room_id);
tm_request *tm_api_room_discover();
tm_request *tm_api_room_create(const char *room_name, const char *room_type);
tm_request *tm_api_room_join(const char *room_id);
tm_request *tm_api_room_invite(const char *room_id,
                               const char *user_credential);
tm_request *tm_api_room_leave(const char *room_id);

tm_request *tm_api_msg_send(const char *room_id, const char *message_data);
tm_request *tm_api_msg_req(const char *room_id);

tm_request *tm_api_link_extract(const char *link);

tm_request *tm_api_status_set(const char *status);
tm_request *tm_api_status_req();

typedef struct
{
    void (*handle)(const char *room_id,
                   const char *room_name,
                   const char *room_type,
                   const char *room_data,
                   void *data);
    void *data;
} tm_api_room_self__callback;

void tm_api_room_self__handle(int argc, char **argv, void *_callback);

tm_handler *tm_api_room_self(tm_api_room_self__callback *callback);

typedef struct
{
    void (*handle)(const char *room_id,
                   const char *action,
                   const char *user_id,
                   const char *user_credential,
                   void *data);
    void *data;
} tm_api_room_any__callback;

void tm_api_room_any__handle(int argc, char **argv, void *_callback);

tm_handler *tm_api_room_any(tm_api_room_any__callback *callback);

typedef struct
{
    void (*handle)(const char *room_id,
                   const char *timestamp,
                   const char *user_id,
                   const char *user_credential,
                   const char *message_data,
                   void *data);
    void *data;
} tm_api_msg_recv__callback;

void tm_api_msg_recv__handle(int argc, char **argv, void *_callback);

tm_handler *tm_api_msg_recv(tm_api_msg_recv__callback *callback);

typedef struct
{
    void (*handle)(const char *user_id,
                   const char *user_credential,
                   const char *status,
                   void *data);
    void *data;
} tm_api_status_recv__callback;

void tm_api_status_recv__handle(int argc, char **argv, void *_callback);

tm_handler *tm_api_status_recv(tm_api_status_recv__callback *callback);

#endif
