#ifndef LOG_H
#define LOG_H

#include <glib.h>
#include <syslog.h>

static const char *LOG_IDENT = "libtjota";
static const int LOG_OPTION = LOG_CONS | LOG_PID;
static const int LOG_FACILITY = LOG_USER;

void tm_log_open();
void tm_log_write(int priority, const char *format, ...);
void tm_log_gerror(GError *error);
void tm_log_close();

#endif
