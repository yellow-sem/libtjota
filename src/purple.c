#define PURPLE_PLUGINS

#include <glib.h>

#include "notify.h"
#include "plugin.h"
#include "version.h"

static gboolean plugin_load(PurplePlugin *plugin)
{
    purple_notify_message(plugin, PURPLE_NOTIFY_MSG_INFO, "Hello World!",
                          "This is the Hello World! plugin :)", NULL, NULL, NULL);
    return TRUE;
}

static PurplePluginInfo info = {
    PURPLE_PLUGIN_MAGIC,
    PURPLE_MAJOR_VERSION,
    PURPLE_MINOR_VERSION,
    PURPLE_PLUGIN_STANDARD,
    NULL,
    0,
    NULL,
    PURPLE_PRIORITY_DEFAULT,

    "core-yellow-tjota",
    "Tjöta",
    "0.1",

    "Tjöta Chat Client",
    "Connect to a Tjöta chat server.",
    "Sebastian Nogara <snogaraleal@gmail.com>",
    "http://tjota.online",
    
    plugin_load,
    NULL,
    NULL,

    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};                               
    
static void init_plugin(PurplePlugin *plugin)
{
}

PURPLE_INIT_PLUGIN(hello_world, init_plugin, info)
