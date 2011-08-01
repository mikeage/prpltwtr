#ifndef _TWITTER_ENDPOINT_CHAT_H_
#define _TWITTER_ENDPOINT_CHAT_H_

#include "defaults.h"

#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <glib.h>

#include <account.h>
#include <accountopt.h>
#include <blist.h>
#include <cmds.h>
#include <conversation.h>
#include <connection.h>
#include <core.h>
#include <debug.h>
#include <notify.h>
#include <privacy.h>
#include <prpl.h>
#include <roomlist.h>
#include <status.h>
#include <util.h>
#include <version.h>
#include <cipher.h>
#include <sslconn.h>
#include <request.h>

#include "prpltwtr_util.h"
#include "prpltwtr_conn.h"

typedef enum {
    TWITTER_CHAT_SEARCH = 0,
    TWITTER_CHAT_TIMELINE = 1,
    TWITTER_CHAT_LIST = 2,
    TWITTER_CHAT_UNKNOWN = 3                     //keep this as the last element
} TwitterChatType;

typedef struct _TwitterEndpointChatSettings TwitterEndpointChatSettings;
typedef struct _TwitterEndpointChat TwitterEndpointChat;
typedef void    (*TwitterChatLeaveFunc) (TwitterEndpointChat * ctx);
typedef         gint(*TwitterChatSendMessageFunc) (TwitterEndpointChat * ctx, const char *message);

struct _TwitterEndpointChatSettings {
    TwitterChatType type;
#ifdef _HAZE_
    gchar           conv_id;
#endif
    gchar          *(*get_status_added_text) (TwitterEndpointChat * endpoint_chat);
    //int (*send_message)(TwitterEndpointChat *endpoint_chat, const gchar *message);
    void            (*endpoint_data_free) (gpointer endpoint_data);
                    gint(*get_default_interval) (PurpleAccount * account);
    gchar          *(*get_name) (GHashTable * components);
    gchar          *(*verify_components) (GHashTable * components);
                    gboolean(*interval_timeout) (TwitterEndpointChat * endpoint_chat);
                    gboolean(*on_start) (TwitterEndpointChat * endpoint_chat);
                    gpointer(*create_endpoint_data) (GHashTable * components);
};

struct _TwitterEndpointChat {
    TwitterChatType type;
    PurpleAccount  *account;
    guint           timer_handle;
    gchar          *chat_name;
    TwitterEndpointChatSettings *settings;
    gpointer        endpoint_data;

    GList          *sent_tweet_ids;
    int             rate_limit_total;
    int             rate_limit_remaining;
    gboolean        retrieval_in_progress;
};

//Identifier to use for multithreading
//TODO this will (probably) stll cause crashes if an account is destroyed while doing multithreading
typedef struct {
    PurpleAccount  *account;
    gchar          *chat_name;
} TwitterEndpointChatId;

TwitterEndpointChatId *twitter_endpoint_chat_id_new(TwitterEndpointChat * chat);
void            twitter_endpoint_chat_id_free(TwitterEndpointChatId * chat_id);
TwitterEndpointChat *twitter_endpoint_chat_find_by_id(TwitterEndpointChatId * chat_id);

TwitterEndpointChat *twitter_endpoint_chat_new(TwitterEndpointChatSettings * settings, TwitterChatType type, PurpleAccount * account, const gchar * chat_name, GHashTable * components);

void            twitter_endpoint_chat_init(const char *protocol_id);
void            twitter_endpoint_chat_free(TwitterEndpointChat * ctx);

void            twitter_endpoint_chat_start(PurpleConnection * gc, TwitterEndpointChatSettings * settings, GHashTable * components, gboolean open_conv);
TwitterEndpointChat *twitter_endpoint_chat_find(PurpleAccount * account, const char *chat_name);

//TODO: we should replace this with a find by components. What's going on here with HAZE?
PurpleChat     *twitter_blist_chat_find_search(PurpleAccount * account, const char *name);
PurpleChat     *twitter_blist_chat_find_timeline(PurpleAccount * account, gint timeline_id);
PurpleChat     *twitter_blist_chat_find_list(PurpleAccount * account, const char *name);
PurpleChat     *twitter_blist_chat_find(PurpleAccount * account, const char *name);

gboolean        twitter_blist_chat_is_auto_open(PurpleChat * chat);

typedef enum _TWITTER_ATTACH_SEARCH_TEXT {
    TWITTER_ATTACH_SEARCH_TEXT_NONE = 0,
    TWITTER_ATTACH_SEARCH_TEXT_PREPEND = 1,
    TWITTER_ATTACH_SEARCH_TEXT_APPEND = 2,
} TWITTER_ATTACH_SEARCH_TEXT;

TWITTER_ATTACH_SEARCH_TEXT twitter_blist_chat_attach_search_text(PurpleChat * chat);

void            twitter_chat_update_rate_limit(TwitterEndpointChat * endpoint_chat);

void            twitter_chat_got_tweet(TwitterEndpointChat * endpoint_chat, TwitterUserTweet * tweet);
void            twitter_chat_got_user_tweets(TwitterEndpointChat * endpoint_chat, GList * user_tweets);
int             twitter_endpoint_chat_send(TwitterEndpointChat * ctx, const gchar * message);

TwitterEndpointChatSettings *twitter_get_endpoint_chat_settings(TwitterChatType type);

#endif
