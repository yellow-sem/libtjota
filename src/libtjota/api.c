#include "api.h"

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

void tm_api_room_self__handle(int argc, char **argv, void *data)
{
    void (*callback)(const char *room_id,
                     const char *room_name,
                     const char *room_type) = data;

    callback(argv[1], argv[2], argv[3]);
}

tm_handler *tm_api_room_self(void (*callback)(const char *room_id,
                                              const char *room_name,
                                              const char *room_type))
{
    return tm_handler_new_regex("room:self any ([^ ]*) '([^']*)' ([^ ]*)",
                                &tm_api_room_self__handle,
                                callback);
}
