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

tm_request *tm_api_msg_send(const char *room_id, const char *data);
tm_request *tm_api_msg_req(const char *room_id);

tm_request *tm_api_link_extract(const char *link);

tm_handler *tm_api_room_self(void (*callback)(const char *room_id,
                                              const char *room_name,
                                              const char *room_type));

tm_handler *tm_api_room_any(void (*callback)(const char *room_id,
                                             int type,
                                             const char *user_id,
                                             const char *user_credential));

tm_handler *tm_api_msg_recv(void (*callback)(const char *room_id,
                                             int timestamp,
                                             const char *user_id,
                                             const char *data));

#endif
