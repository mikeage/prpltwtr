#ifndef _TWITTER_API_H_
#define _TWITTER_API_H_

#include "config.h"

#include <account.h>
#include <accountopt.h>
#include <blist.h>
#include <cmds.h>
#include <conversation.h>
#include <connection.h>
#include <debug.h>
#include <notify.h>
#include <privacy.h>
#include <prpl.h>
#include <roomlist.h>
#include <status.h>
#include <util.h>
#include <version.h>
#include <cipher.h>
#include <request.h>

#include "twitter_request.h"
#include "twitter_prefs.h"
#include "twitter_search.h"

typedef void (*TwitterApiMultiStatusSuccessFunc)(PurpleAccount *account, xmlnode *node, gboolean last_page, gpointer user_data);
typedef gboolean (*TwitterApiMultiStatusErrorFunc)(PurpleAccount *account, const TwitterRequestErrorData *error_data, gpointer user_data);

void twitter_api_get_friends(PurpleAccount *account,
		TwitterSendRequestMultiPageAllSuccessFunc success_func,
		TwitterSendRequestMultiPageAllErrorFunc error_func,
		gpointer data);

void twitter_api_get_home_timeline_all(PurpleAccount *account,
		long long since_id,
		TwitterSendRequestMultiPageAllSuccessFunc success_func,
		TwitterSendRequestMultiPageAllErrorFunc error_func,
		gint max_count,
		gpointer data);

void twitter_api_get_home_timeline(PurpleAccount *account,
		long long since_id,
		int count,
		int page,
		TwitterSendXmlRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data);

void twitter_api_get_dms(PurpleAccount *account,
		long long since_id,
		int count,
		int page,
		TwitterSendXmlRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data);

void twitter_api_get_dms_all(PurpleAccount *account,
		long long since_id,
		TwitterSendRequestMultiPageAllSuccessFunc success_func,
		TwitterSendRequestMultiPageAllErrorFunc error_func,
		gint max_count,
		gpointer data);

void twitter_api_get_replies_all(PurpleAccount *account,
		long long since_id,
		TwitterSendRequestMultiPageAllSuccessFunc success_func,
		TwitterSendRequestMultiPageAllErrorFunc error_func,
		gint max_count,
		gpointer data);

void twitter_api_get_replies(PurpleAccount *account,
		long long since_id,
		int count,
		int page,
		TwitterSendXmlRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data);

void twitter_api_get_rate_limit_status(PurpleAccount *account,
		TwitterSendXmlRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data);

void twitter_api_send_dm(PurpleAccount *account,
		const char *user,
		const char *msg,
		TwitterSendXmlRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data);

void twitter_api_set_statuses(PurpleAccount *account,
		GArray *statuses,
		long long in_reply_to_status_id,
		TwitterApiMultiStatusSuccessFunc success_func,
		TwitterApiMultiStatusErrorFunc error_func,
		gpointer data);

void twitter_api_send_dms(PurpleAccount *account,
		const gchar *who,
		GArray *statuses,
		TwitterApiMultiStatusSuccessFunc success_func,
		TwitterApiMultiStatusErrorFunc error_func,
		gpointer data);

void twitter_api_set_status(PurpleAccount *account,
		const char *msg,
		long long in_reply_to_status_id,
		TwitterSendXmlRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data);

void twitter_api_get_saved_searches (PurpleAccount *account,
        TwitterSendXmlRequestSuccessFunc success_func,
        TwitterSendRequestErrorFunc error_func,
        gpointer data);

/**
 * @rpp: The number of tweets to return per page, up to a max of 100
 */
void twitter_api_search (PurpleAccount *account,
        const char *keyword,
        long long since_id,
        guint rpp,
        TwitterSearchSuccessFunc success_func,
        TwitterSearchErrorFunc error_func,
        gpointer data);

void twitter_api_search_refresh (PurpleAccount *account,
        const char *refresh_url,
        TwitterSearchSuccessFunc success_func,
        TwitterSearchErrorFunc error_func,
        gpointer data);

void twitter_api_verify_credentials(PurpleAccount *account,
		TwitterSendXmlRequestSuccessFunc success_cb,
		TwitterSendRequestErrorFunc error_cb,
		gpointer user_data);

void twitter_api_oauth_request_token(PurpleAccount *account,
		TwitterSendRequestSuccessFunc success_cb,
		TwitterSendRequestErrorFunc error_cb,
		gpointer user_data);

void twitter_api_oauth_access_token(PurpleAccount *account,
		const gchar *oauth_verifier,
		TwitterSendRequestSuccessFunc success_cb,
		TwitterSendRequestErrorFunc error_cb,
		gpointer user_data);

#endif
