/**
 * Copyright (C) 2009 Tan Miaoqing
 * Contact: Tan Miaoqing <rabbitrun84@gmail.com>
 */

#ifndef _TWITTER_UTIL_H_
#define _TWITTER_UTIL_H_

#include <string.h>
#include <glib.h>
#include <xmlnode.h>
#include <debug.h>

#include "config.h"
#include "twitter_xml.h"
#include "twitter_prefs.h" //TODO move

#define TWITTER_URI_ACTION_USER		"user" //TODO: move?
#define TWITTER_URI_ACTION_SEARCH	"search" //TODO: move?




long long purple_account_get_long_long(PurpleAccount *account, const gchar *key, long long default_value);
void purple_account_set_long_long(PurpleAccount *account, const gchar *key, long long value);

#ifndef g_slice_new0
#define g_slice_new0(a) g_new0(a, 1)
#define g_slice_free(a, b) g_free(b)
#endif

#ifndef g_strcmp0
#define g_strcmp0(a, b) (a == NULL && b == NULL ? 0 : a == NULL ? -1 : b == NULL ? 1 : strcmp(a, b))
#endif

//TODO: move this?
char *twitter_format_tweet(PurpleAccount *account, const char *src_user, const char *message, long long id);
#endif /* UTIL_H_ */
