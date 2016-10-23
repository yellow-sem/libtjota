#include "log.h"

void tm_log_open()
{
    openlog(LOG_IDENT, LOG_OPTION, LOG_FACILITY);
}

void tm_log_write(int priority, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vsyslog(priority, format, args);
    va_end(args);
}

void tm_log_gerror(GError *error)
{
    tm_log_write(LOG_ERR, "glib error: %s", error->message);
}

void tm_log_close()
{
    closelog();
}
