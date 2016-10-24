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
