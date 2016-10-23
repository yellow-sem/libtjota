#ifndef API_H
#define API_H

#include "request.h"
#include "handler.h"

tm_request *tm_api_sys_exit();

tm_request *tm_api_auth_login_credential(char *credential,
                                         char *password);
tm_request *tm_api_auth_login_session(char *session);
tm_request *tm_api_auth_logout(char *session);
tm_request *tm_api_auth_check(char *credential);

tm_request *tm_api_room_list();
tm_request *tm_api_room_list_room(char *room_id);
tm_request *tm_api_room_discover();
tm_request *tm_api_room_create(char *room_name, char *room_type);
tm_request *tm_api_room_join(char *room_id);
tm_request *tm_api_room_invite(char *room_id, char *user_credential);
tm_request *tm_api_room_leave(char *room_id);

tm_request *tm_api_msg_send(char *room_id, char *data);
tm_request *tm_api_msg_req(char *room_id);

tm_request *tm_api_link_extract(char *link);

tm_handler *tm_api_room_self(void (*)(char *room_id,
                                      char *room_name,
                                      char *room_type));

tm_handler *tm_api_room_any(void (*)(char *room_id,
                                     int type,
                                     char *user_id,
                                     char *user_credential));

tm_handler *tm_api_msg_recv(void (*)(char *room_id,
                                     int timestamp,
                                     char *user_id,
                                     char *data));

#endif
