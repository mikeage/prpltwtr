#include "config.h"

#include "pt_prefs.h"

gboolean pt_prefs_get_use_https(PurpleAccount * account)
{
    return purple_account_get_bool(account, PT_PREF_USE_HTTPS, PT_PREF_USE_HTTPS_DEFAULT);
}
#if 0
const gchar    *twitter_option_alias_format(PurpleAccount * account)
{
    return purple_account_get_string(account, TWITTER_PREF_ALIAS_FORMAT, TWITTER_PREF_ALIAS_FORMAT_DEFAULT);
}

gboolean twitter_option_add_link_to_tweet(PurpleAccount * account)
{
    return purple_account_get_bool(account, TWITTER_PREF_ADD_URL_TO_TWEET, TWITTER_PREF_ADD_URL_TO_TWEET_DEFAULT);
}

gint twitter_option_search_timeout(PurpleAccount * account)
{
    return purple_account_get_int(account, TWITTER_PREF_SEARCH_TIMEOUT, TWITTER_PREF_SEARCH_TIMEOUT_DEFAULT);
}

gint twitter_option_timeline_timeout(PurpleAccount * account)
{
    return purple_account_get_int(account, TWITTER_PREF_TIMELINE_TIMEOUT, TWITTER_PREF_TIMELINE_TIMEOUT_DEFAULT);
}

gint twitter_option_list_timeout(PurpleAccount * account)
{
    return purple_account_get_int(account, TWITTER_PREF_LIST_TIMEOUT, TWITTER_PREF_LIST_TIMEOUT_DEFAULT);
}

const gchar    *twitter_option_list_group(PurpleAccount * account)
{
    //TODO: create an option for this
    return TWITTER_PREF_DEFAULT_LIST_GROUP;
}

const gchar    *twitter_option_search_group(PurpleAccount * account)
{
    //TODO: create an option for this
    return TWITTER_PREF_DEFAULT_SEARCH_GROUP;
}

const gchar    *twitter_option_buddy_group(PurpleAccount * account)
{
    //TODO: create an option for this
    return TWITTER_PREF_DEFAULT_BUDDY_GROUP;
}

gint twitter_option_replies_timeout(PurpleAccount * account)
{
    return purple_account_get_int(account, TWITTER_PREF_REPLIES_TIMEOUT, TWITTER_PREF_REPLIES_TIMEOUT_DEFAULT);
}

gint twitter_option_dms_timeout(PurpleAccount * account)
{
    return purple_account_get_int(account, TWITTER_PREF_DMS_TIMEOUT, TWITTER_PREF_DMS_TIMEOUT_DEFAULT);
}

gint twitter_option_user_status_timeout(PurpleAccount * account)
{
    return purple_account_get_int(account, TWITTER_PREF_USER_STATUS_TIMEOUT, TWITTER_PREF_USER_STATUS_TIMEOUT_DEFAULT);
}

gboolean twitter_option_get_following(PurpleAccount * account)
{
    return purple_account_get_bool(account, TWITTER_PREF_GET_FRIENDS, TWITTER_PREF_GET_FRIENDS_DEFAULT);
}

gboolean twitter_option_get_history(PurpleAccount * account)
{
    return purple_account_get_bool(account, TWITTER_PREF_RETRIEVE_HISTORY, TWITTER_PREF_RETRIEVE_HISTORY_DEFAULT);
}

gboolean twitter_option_sync_status(PurpleAccount * account)
{
    return purple_account_get_bool(account, TWITTER_PREF_SYNC_STATUS, TWITTER_PREF_SYNC_STATUS_DEFAULT);
}

gboolean twitter_option_use_https(PurpleAccount * account)
{
    return purple_account_get_bool(account, TWITTER_PREF_USE_HTTPS, TWITTER_PREF_USE_HTTPS_DEFAULT);
}

gboolean twitter_option_use_oauth(PurpleAccount * account)
{
    if (!strcmp(purple_account_get_protocol_id(account), TWITTER_PROTOCOL_ID)) {
        return TRUE;
    } else {
        return purple_account_get_bool(account, TWITTER_PREF_USE_OAUTH, TWITTER_PREF_USE_OAUTH_DEFAULT);
    }
}

gint twitter_option_home_timeline_max_tweets(PurpleAccount * account)
{
    return purple_account_get_int(account, TWITTER_PREF_HOME_TIMELINE_MAX_TWEETS, TWITTER_PREF_HOME_TIMELINE_MAX_TWEETS_DEFAULT);
}

gint twitter_option_list_max_tweets(PurpleAccount * account)
{
    return purple_account_get_int(account, TWITTER_PREF_HOME_TIMELINE_MAX_TWEETS, TWITTER_PREF_HOME_TIMELINE_MAX_TWEETS_DEFAULT);
}

gboolean twitter_option_default_dm(PurpleAccount * account)
{
    return purple_account_get_bool(account, TWITTER_PREF_DEFAULT_DM, TWITTER_PREF_DEFAULT_DM_DEFAULT);
}

static const gchar *twitter_get_host_from_base(const gchar * base)
{
    static gchar    host[256];
    gchar          *slash = strchr(base, '/');
    int             len = slash ? slash - base : strlen(base);
    if (len > 255)
        return NULL;
    strncpy(host, base, len);
    host[len] = '\0';
    return host;
}

static const gchar *twitter_get_subdir_from_base(const gchar * base)
{
    if (!base)
        return NULL;
    return strchr(base, '/');
}

const gchar    *twitter_option_api_host(PurpleAccount * account)
{
    if (!strcmp(purple_account_get_protocol_id(account), TWITTER_PROTOCOL_ID)) {
        return twitter_get_host_from_base(purple_account_get_string(account, TWITTER_PREF_API_BASE, TWITTER_PREF_API_BASE_DEFAULT));
    } else {
        return twitter_get_host_from_base(purple_account_get_string(account, TWITTER_PREF_API_BASE, STATUSNET_PREF_API_BASE_DEFAULT));
    }
}

const gchar    *twitter_option_api_subdir(PurpleAccount * account)
{
    if (!strcmp(purple_account_get_protocol_id(account), TWITTER_PROTOCOL_ID)) {
        return twitter_get_subdir_from_base(purple_account_get_string(account, TWITTER_PREF_API_BASE, TWITTER_PREF_API_BASE_DEFAULT));
    } else {
        return twitter_get_subdir_from_base(purple_account_get_string(account, TWITTER_PREF_API_BASE, STATUSNET_PREF_API_BASE_DEFAULT));
    }
}

const gchar    *twitter_option_web_host(PurpleAccount * account)
{
    return twitter_get_host_from_base(purple_account_get_string(account, TWITTER_PREF_WEB_BASE, TWITTER_PREF_WEB_BASE_DEFAULT));
}

const gchar    *twitter_option_web_subdir(PurpleAccount * account)
{
    return twitter_get_subdir_from_base(purple_account_get_string(account, TWITTER_PREF_WEB_BASE, TWITTER_PREF_WEB_BASE_DEFAULT));
}

const gchar    *twitter_option_search_api_host(PurpleAccount * account)
{
    if (!strcmp(purple_account_get_protocol_id(account), TWITTER_PROTOCOL_ID)) {
        return twitter_get_host_from_base(purple_account_get_string(account, TWITTER_PREF_SEARCH_API_BASE, TWITTER_PREF_SEARCH_API_BASE_DEFAULT));
    } else {
        return twitter_get_host_from_base(purple_account_get_string(account, TWITTER_PREF_SEARCH_API_BASE, STATUSNET_PREF_API_BASE_DEFAULT));
    }
}

const gchar    *twitter_option_search_api_subdir(PurpleAccount * account)
{
    if (!strcmp(purple_account_get_protocol_id(account), TWITTER_PROTOCOL_ID)) {
        return twitter_get_subdir_from_base(purple_account_get_string(account, TWITTER_PREF_SEARCH_API_BASE, TWITTER_PREF_SEARCH_API_BASE_DEFAULT));
    } else {
        return twitter_get_subdir_from_base(purple_account_get_string(account, TWITTER_PREF_SEARCH_API_BASE, STATUSNET_PREF_API_BASE_DEFAULT));
    }
}

int twitter_option_cutoff_time(PurpleAccount * account)
{
    return purple_account_get_int(account, TWITTER_ONLINE_CUTOFF_TIME_HOURS, TWITTER_ONLINE_CUTOFF_TIME_HOURS_DEFAULT);
}
#endif
