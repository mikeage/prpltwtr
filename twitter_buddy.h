#ifndef _TWITTER_BUDDY_H_
#define _TWITTER_BUDDY_H_
#include "twitter_xml.h"
#include "twitter_prefs.h"

TwitterUserTweet *twitter_buddy_get_buddy_data(PurpleBuddy *b);
void twitter_buddy_set_status_data(PurpleAccount *account, const char *src_user, TwitterTweet *s);
TwitterUserTweet *twitter_buddy_get_buddy_data(PurpleBuddy *b);
PurpleBuddy *twitter_buddy_new(PurpleAccount *account, const char *screenname, const char *alias);
void twitter_buddy_set_user_data(PurpleAccount *account, TwitterUserData *u, gboolean add_missing_buddy);
void twitter_buddy_update_icon_cb(PurpleUtilFetchUrlData *url_data, gpointer user_data, const gchar *url_text, gsize len, const gchar *error_message);
void twitter_buddy_update_icon(PurpleBuddy *buddy);

#endif
