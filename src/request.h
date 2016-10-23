#ifndef REQUEST_H
#define REQUEST_H

struct tm_request
{
    char *command;
    char *ident;
    int argc;
    char **argv;
};

struct tm_request *tm_request_new();
struct tm_request *tm_request_new_command(char *command);
struct tm_request *tm_request_new_command_args(char *command, int argc, ...);
void tm_request_free(struct tm_request *request);

char *tm_request_encode(struct tm_request *request);

#endif
