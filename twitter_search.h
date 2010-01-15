/**
 * Copyright (C) 2009 Tan Miaoqing
 * Contact: Tan Miaoqing <rabbitrun84@gmail.com>
 */

#ifndef _TWITTER_SEARCH_H_
#define _TWITTER_SEARCH_H_

#include <glib.h>
#include "twitter_xml.h"

typedef struct _TwitterSearchErrorData TwitterSearchErrorData;

struct _TwitterSearchErrorData
{

};

/* @search_result: an array of TwitterUserTweet */
typedef void (*TwitterSearchSuccessFunc)(PurpleAccount *acct,
        const GArray *search_results,
        const gchar *refresh_url,
        long long max_id,
        gpointer user_data);

typedef gboolean (*TwitterSearchErrorFunc)(PurpleAccount *acct,
        const TwitterSearchErrorData *error_data,
        gpointer user_data);

void twitter_search (PurpleAccount *account, const char *query,
        TwitterSearchSuccessFunc success_cb, TwitterSearchErrorFunc error_cb,
        gpointer data);

#endif /* TWITTER_SEARCH_H_ */
