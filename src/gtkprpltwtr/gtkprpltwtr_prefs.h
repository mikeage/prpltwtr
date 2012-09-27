#ifndef _GTKPRPLTWTR_PREFS_H_
#define _GTKPRPLTWTR_PREFS_

#include "gtkprpltwtr.h"
#include <plugin.h>
#include <gtkplugin.h>

#define PREF_PREFIX "/plugins/gtk/" PLUGIN_ID

#define TWITTER_PREF_ENABLE_CONV_ICON PREF_PREFIX "/enable_conv_icon"
#define TWITTER_PREF_ENABLE_CONV_ICON_DEFAULT TRUE
#define TWITTER_PREF_CONV_ICON_SIZE PREF_PREFIX "/conv_icon_size"
#define TWITTER_PREF_CONV_ICON_SIZE_DEFAULT 32

void            gtkprpltwtr_prefs_init(PurplePlugin * plugin);
void            gtkprpltwtr_prefs_destroy(PurplePlugin * plugin);

#endif
