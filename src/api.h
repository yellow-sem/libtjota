#ifndef API_H
#define API_H

#include "request.h"
#include "handler.h"

struct tm_request *tm_api_sys_exit();

struct tm_request *tm_api_auth_login_credential(char *credential,
                                                char *password);
struct tm_request *tm_api_auth_login_session(char *session);
struct tm_request *tm_api_auth_logout(char *session);
struct tm_request *tm_api_auth_check(char *credential);

struct tm_request *tm_api_room_list();
struct tm_request *tm_api_room_list_room(char *room_id);
struct tm_request *tm_api_room_discover();
struct tm_request *tm_api_room_create(char *room_name, char *room_type);
struct tm_request *tm_api_room_join(char *room_id);
struct tm_request *tm_api_room_invite(char *room_id, char *user_credential);
struct tm_request *tm_api_room_leave(char *room_id);

struct tm_request *tm_api_msg_send(char *room_id, char *data);
struct tm_request *tm_api_msg_req(char *room_id);

struct tm_request *tm_api_link_extract(char *link);

struct tm_handler *tm_api_room_self(void (*)(char *room_id,
                                             char *room_name,
                                             char *room_type));

struct tm_handler *tm_api_room_any(void (*)(char *room_id,
                                            int type,
                                            char *user_id,
                                            char *user_credential));

struct tm_handler *tm_api_msg_recv(void (*)(char *room_id,
                                            int timestamp,
                                            char *user_id,
                                            char *data));

#endif
