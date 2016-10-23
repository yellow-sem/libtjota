#include "api.h"

struct tm_request *tm_api_sys_exit()
{
    return tm_request_new_command("sys:exit");
}

struct tm_request *tm_api_auth_login_credential(char *credential,
                                                char *password)
{
    return tm_request_new_command_args("auth:login", 2, credential, password);
}
