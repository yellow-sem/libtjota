#include "api.h"

#include <stdlib.h>

tm_request *tm_api_sys_exit()
{
    return tm_request_new_command("sys:exit");
}

tm_request *tm_api_auth_login_credential(const char *credential,
                                         const char *password)
{
    return tm_request_new_command_args("auth:login", 2, credential, password);
}

tm_request *tm_api_auth_login_session(const char *session)
{
    return tm_request_new_command_args("auth:login", 1, session);
}

tm_request *tm_api_room_list()
{
    return tm_request_new_command("room:list");
}

tm_request *tm_api_room_list_room(const char *room_id)
{
    return tm_request_new_command_args("room:list", 1, room_id);
}

tm_request *tm_api_msg_send(const char *room_id, const char *message_data)
{
    return tm_request_new_command_args("msg:send", 2, room_id, message_data);
}

tm_request *tm_api_msg_req(const char *room_id)
{
    return tm_request_new_command_args("msg:req", 1, room_id);
}

void tm_api_room_self__handle(int argc, char **argv, void *_callback)
{
    tm_api_room_self__callback *callback = _callback;

    callback->handle(argv[1], argv[2], argv[3], callback->data);
}

tm_handler *tm_api_room_self(tm_api_room_self__callback *callback)
{
    return tm_handler_new_regex("room:self any ([^ ]*) '([^']*)' ([^ ]*)",
                                &tm_api_room_self__handle,
                                callback);
}

void tm_api_room_any__handle(int argc, char **argv, void *_callback)
{
    tm_api_room_any__callback *callback = _callback;

    callback->handle(argv[1], argv[2], argv[3], argv[4], callback->data);
}

tm_handler *tm_api_room_any(tm_api_room_any__callback *callback)
{
    return tm_handler_new_regex("room:\\* any ([^ ]*) ([^ ]*) ([^ ]*) ([^ ]*)",
                                &tm_api_room_any__handle,
                                callback);
}

void tm_api_msg_recv__handle(int argc, char **argv, void *_callback)
{
    tm_api_msg_recv__callback *callback = _callback;

    callback->handle(argv[1], argv[2], argv[3], argv[4], argv[5],
                     callback->data);
}

tm_handler *tm_api_msg_recv(tm_api_msg_recv__callback *callback)
{
    return tm_handler_new_regex(
            "msg:recv any ([^ ]*) ([^ ]*) ([^ ]*) ([^ ]*) '([^']*)'",
            &tm_api_msg_recv__handle,
            callback);
}
