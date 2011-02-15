#ifndef _TWITTER_CHARCOUNT_H_
#define _TWITTER_CHARCOUNT_H_

#include "defaults.h"
#include <glib.h>
#include <conversation.h>

void twitter_charcount_detach_from_all_windows(void);
void twitter_charcount_attach_to_all_windows(void);
void twitter_charcount_conv_created_cb(PurpleConversation *conv, gpointer null);
void twitter_charcount_conv_destroyed_cb(PurpleConversation *conv, gpointer null);

#endif
