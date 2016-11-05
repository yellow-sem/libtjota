#ifndef REQUEST_H
#define REQUEST_H

typedef struct
{
    char *command;
    char *ident;
    int argc;
    char **argv;
} tm_request;

tm_request *tm_request_new();
tm_request *tm_request_new_command(char *command);
tm_request *tm_request_new_command_args(char *command, int argc, ...);
void tm_request_free(tm_request *request);

char *tm_request_format(tm_request *request);

#endif
