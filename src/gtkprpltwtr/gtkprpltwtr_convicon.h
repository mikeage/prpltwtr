#ifndef _TWITTER_CONVICON_H_
#define _TWITTER_CONVICON_H_

#include "defaults.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <sys/stat.h>
#include <time.h>
#include <locale.h>

#include <gdk-pixbuf/gdk-pixbuf.h>

#include <gtkplugin.h>
#include <util.h>
#include <debug.h>
#include <connection.h>
#include <version.h>
#include <sound.h>
#include <gtkconv.h>
#include <gtkimhtml.h>
#include <core.h>

typedef struct {
    GdkPixbuf      *pixbuf;  /* icon pixmap */
    gboolean        requested;  /* TRUE if download icon has been requested */
    GList          *request_list;   /* marker list */
    PurpleUtilFetchUrlData *fetch_data; /* icon fetch data */
    gchar          *icon_url;   /* url for the user's icon */
    time_t          mtime;   /* mtime of file */
    GList          *convs;   /* list of conversations */

    gchar          *username;
} TwitterConvIcon;

void            twitter_conv_icon_account_load(PurpleAccount * account);
void            twitter_conv_icon_account_unload(PurpleAccount * account);

/* Call when icon data is received for a user
 * Make sure this is only called right before displaying a chat message
 */
void            twitter_conv_icon_got_user_icon(PurpleAccount * account, const char *user_name, const gchar * url, time_t icon_time);
void            twitter_conv_icon_got_buddy_icon(PurpleAccount * account, const char *user_name, PurpleBuddyIcon * buddy_icon);

#endif
