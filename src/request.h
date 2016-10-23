#ifndef REQUEST_H
#define REQUEST_H

struct tm_request
{
    char *command;
    char *ident;
    int argc;
    char **argv;
};

char *tm_request_encode(struct tm_request *request);

#endif
