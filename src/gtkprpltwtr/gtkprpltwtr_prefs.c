#include "defaults.h"
#include "gtkprpltwtr_prefs.h"

void gtkprpltwtr_prefs_destroy(PurplePlugin * plugin)
{
}

void gtkprpltwtr_prefs_init(PurplePlugin * plugin)
{
    purple_prefs_add_none(PREF_PREFIX);
    purple_prefs_add_int(TWITTER_PREF_CONV_ICON_SIZE, TWITTER_PREF_CONV_ICON_SIZE_DEFAULT);
    purple_prefs_add_bool(TWITTER_PREF_ENABLE_CONV_ICON, TWITTER_PREF_ENABLE_CONV_ICON_DEFAULT);
}
