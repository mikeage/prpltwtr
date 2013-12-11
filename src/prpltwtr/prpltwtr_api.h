#ifndef _TWITTER_API_H_
#define _TWITTER_API_H_

#include "defaults.h"

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

#include "prpltwtr_request.h"
#include "prpltwtr_prefs.h"
#include "prpltwtr_search.h"

const gchar *twitter_api_create_url(PurpleAccount * account, const gchar * endpoint);
const gchar *twitter_api_create_url_ext(PurpleAccount * account, const gchar * endpoint, const gchar * extension);

typedef void    (*TwitterApiMultiStatusSuccessFunc) (PurpleAccount * account, gpointer node, gboolean last_page, gpointer user_data);
typedef         gboolean(*TwitterApiMultiStatusErrorFunc) (PurpleAccount * account, const TwitterRequestErrorData * error_data, gpointer user_data);

void            twitter_api_get_friends(TwitterRequestor * r, TwitterSendRequestMultiPageAllSuccessFunc success_func, TwitterSendRequestMultiPageAllErrorFunc error_func, gpointer data);

void            twitter_api_get_home_timeline_all(TwitterRequestor * r, gchar * since_id, TwitterSendRequestMultiPageAllSuccessFunc success_func, TwitterSendRequestMultiPageAllErrorFunc error_func, gint max_count, gpointer data);

void            twitter_api_get_home_timeline(TwitterRequestor * r, gchar * since_id, int count, int page, TwitterSendFormatRequestSuccessFunc success_func, TwitterSendRequestErrorFunc error_func, gpointer data);

void            twitter_api_get_list(TwitterRequestor * r, const gchar * list_id, const gchar * owner, gchar * since_id, int count, int page, TwitterSendFormatRequestSuccessFunc success_func, TwitterSendRequestErrorFunc error_func, gpointer data);

void            twitter_api_get_list_all(TwitterRequestor * r, const gchar * list_id, const gchar * owner, gchar * since_id, TwitterSendRequestMultiPageAllSuccessFunc success_func, TwitterSendRequestMultiPageAllErrorFunc error_func, gint max_count, gpointer data);

void            twitter_api_get_dms(TwitterRequestor * r, gchar * since_id, int count, int page, TwitterSendFormatRequestSuccessFunc success_func, TwitterSendRequestErrorFunc error_func, gpointer data);

void            twitter_api_get_dms_all(TwitterRequestor * r, gchar * since_id, TwitterSendRequestMultiPageAllSuccessFunc success_func, TwitterSendRequestMultiPageAllErrorFunc error_func, gint max_count, gpointer data);

void            twitter_api_get_replies_all(TwitterRequestor * r, gchar * since_id, TwitterSendRequestMultiPageAllSuccessFunc success_func, TwitterSendRequestMultiPageAllErrorFunc error_func, gint max_count, gpointer data);

void            twitter_api_get_replies(TwitterRequestor * r, gchar * since_id, int count, int page, TwitterSendFormatRequestSuccessFunc success_func, TwitterSendRequestErrorFunc error_func, gpointer data);

void            twitter_api_get_rate_limit_status(TwitterRequestor * r, TwitterSendFormatRequestSuccessFunc success_func, TwitterSendRequestErrorFunc error_func, gpointer data);

void            twitter_api_send_dm(TwitterRequestor * r, const char *user, const char *msg, TwitterSendFormatRequestSuccessFunc success_func, TwitterSendRequestErrorFunc error_func, gpointer data);

void            twitter_api_send_rt(TwitterRequestor * r, gchar * id, TwitterSendFormatRequestSuccessFunc success_func, TwitterSendRequestErrorFunc error_func, gpointer data);

void            twitter_api_delete_favorite(TwitterRequestor * r, gchar * id, TwitterSendFormatRequestSuccessFunc success_func, TwitterSendRequestErrorFunc error_func, gpointer data);

void            twitter_api_add_favorite(TwitterRequestor * r, gchar * id, TwitterSendFormatRequestSuccessFunc success_func, TwitterSendRequestErrorFunc error_func, gpointer data);

void            twitter_api_report_spammer(TwitterRequestor * r, const gchar * user, TwitterSendFormatRequestSuccessFunc success_func, TwitterSendRequestErrorFunc error_func, gpointer data);

void            twitter_api_get_status(TwitterRequestor * r, gchar * id, TwitterSendFormatRequestSuccessFunc success_func, TwitterSendRequestErrorFunc error_func, gpointer data);

void            twitter_api_delete_status(TwitterRequestor * r, gchar * id, TwitterSendFormatRequestSuccessFunc success_func, TwitterSendRequestErrorFunc error_func, gpointer data);

void            twitter_api_set_statuses(TwitterRequestor * r, GArray * statuses, gchar * in_reply_to_status_id, TwitterApiMultiStatusSuccessFunc success_func, TwitterApiMultiStatusErrorFunc error_func, gpointer data);

void            twitter_api_send_dms(TwitterRequestor * r, const gchar * who, GArray * statuses, TwitterApiMultiStatusSuccessFunc success_func, TwitterApiMultiStatusErrorFunc error_func, gpointer data);

void            twitter_api_set_status(TwitterRequestor * r, const char *msg, gchar * in_reply_to_status_id, TwitterSendFormatRequestSuccessFunc success_func, TwitterSendRequestErrorFunc error_func, gpointer data);

void            twitter_api_get_personal_lists(TwitterRequestor * r, TwitterSendFormatRequestSuccessFunc success_func, TwitterSendRequestErrorFunc error_func, gpointer data);
void            twitter_api_get_subscribed_lists(TwitterRequestor * r, TwitterSendFormatRequestSuccessFunc success_func, TwitterSendRequestErrorFunc error_func, gpointer data);

void            twitter_api_get_saved_searches(TwitterRequestor * r, TwitterSendFormatRequestSuccessFunc success_func, TwitterSendRequestErrorFunc error_func, gpointer data);

void            twitter_api_web_open_favorites(PurplePluginAction * action);
void            twitter_api_web_open_retweets_of_mine(PurplePluginAction * action);
void            twitter_api_web_open_suggested_friends(PurplePluginAction * action);
void            twitter_api_web_open_replies(PurplePluginAction * action);
void            prpltwtr_api_refresh_user(TwitterRequestor * r, const char *username, TwitterSendFormatRequestSuccessFunc success_func, TwitterSendRequestErrorFunc error_func);

/**
 * @rpp: The number of tweets to return per page, up to a max of 100
 */
void            twitter_api_search(TwitterRequestor * r, const char *keyword, gchar * since_id, guint rpp, TwitterSearchSuccessFunc success_func, TwitterSearchErrorFunc error_func, gpointer data);

void            twitter_api_search_refresh(TwitterRequestor * r, const char *refresh_url, TwitterSearchSuccessFunc success_func, TwitterSearchErrorFunc error_func, gpointer data);

void            twitter_api_verify_credentials(TwitterRequestor * r, TwitterSendFormatRequestSuccessFunc success_cb, TwitterSendRequestErrorFunc error_cb, gpointer user_data);

void            twitter_api_get_info(PurpleConnection * gc, const char *username);
#endif
