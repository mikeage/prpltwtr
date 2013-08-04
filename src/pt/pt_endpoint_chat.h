#ifndef _PT_ENDPOINT_CHAT_H_
#define _PT_ENDPOINT_CHAT_H_

#include "defines.h"

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

#include "pt_connection.h"

typedef enum {
    PT_CHAT_SEARCH = 0,
    PT_CHAT_TIMELINE = 1,
    PT_CHAT_LIST = 2,
    PT_CHAT_UNKNOWN = 3                     //keep this as the last element
} PtChatType;

typedef struct _PtEndpointChatSettings PtEndpointChatSettings;
typedef struct _PtEndpointChat PtEndpointChat;
typedef void    (*PtChatLeaveFunc) (PtEndpointChat * ctx);
typedef         gint(*PtChatSendMessageFunc) (PtEndpointChat * ctx, const char *message);

struct _PtEndpointChatSettings {
    PtChatType type;
    gchar          *(*get_status_added_text) (PtEndpointChat * endpoint_chat);
    void            (*endpoint_data_free) (gpointer endpoint_data);
                    gint(*get_default_interval) (PurpleAccount * account);
    gchar          *(*get_name) (GHashTable * components);
    gchar          *(*verify_components) (GHashTable * components);
                    gboolean(*interval_timeout) (PtEndpointChat * endpoint_chat);
                    gboolean(*on_start) (PtEndpointChat * endpoint_chat);
                    gpointer(*create_endpoint_data) (GHashTable * components);
};

struct _PtEndpointChat {
    PtChatType type;
    PurpleAccount  *account;
    guint           timer_handle;
    gchar          *chat_name;
    PtEndpointChatSettings *settings;
    gpointer        endpoint_data;

    GList          *sent_tweet_ids;
    gboolean        retrieval_in_progress;
    int             retrieval_in_progress_timeout;  /* Prevent getting stuck */
};

typedef struct {
    PurpleAccount  *account;
    gchar          *chat_name;
} PtEndpointChatId;

PtEndpointChatId *pt_endpoint_chat_id_new(PtEndpointChat * chat);
void            pt_endpoint_chat_id_free(PtEndpointChatId * chat_id);
PtEndpointChat *pt_endpoint_chat_find_by_id(PtEndpointChatId * chat_id);

PtEndpointChat *pt_endpoint_chat_new(PtEndpointChatSettings * settings, PtChatType type, PurpleAccount * account, const gchar * chat_name, GHashTable * components);

void            pt_endpoint_chat_init(const char *protocol_id);
void            pt_endpoint_chat_free(PtEndpointChat * ctx);

void            pt_endpoint_chat_start(PurpleConnection * gc, PtEndpointChatSettings * settings, GHashTable * components, gboolean open_conv);
PtEndpointChat *pt_endpoint_chat_find(PurpleAccount * account, const char *chat_name);

//TODO: we should replace this with a find by components. What's going on here with HAZE?
PurpleChat     *pt_blist_chat_find_search(PurpleAccount * account, const char *name);
PurpleChat     *pt_blist_chat_find_timeline(PurpleAccount * account, gint timeline_id);
PurpleChat     *pt_blist_chat_find_list(PurpleAccount * account, const char *name);
PurpleChat     *pt_blist_chat_find(PurpleAccount * account, const char *name);

gboolean        pt_blist_chat_is_auto_open(PurpleChat * chat);

typedef enum _PT_ATTACH_SEARCH_TEXT {
    PT_ATTACH_SEARCH_TEXT_NONE = 0,
    PT_ATTACH_SEARCH_TEXT_PREPEND = 1,
    PT_ATTACH_SEARCH_TEXT_APPEND = 2,
} PT_ATTACH_SEARCH_TEXT;

PT_ATTACH_SEARCH_TEXT pt_blist_chat_attach_search_text(PurpleChat * chat);

void            pt_chat_update_rate_limit(PtEndpointChat * endpoint_chat);

//void            pt_chat_got_tweet(PtEndpointChat * endpoint_chat, PtUserTweet * tweet);
void            pt_chat_got_user_tweets(PtEndpointChat * endpoint_chat, GList * user_tweets);
int             pt_endpoint_chat_send(PtEndpointChat * ctx, const gchar * message);

PtEndpointChatSettings *pt_get_endpoint_chat_settings(PtChatType type);

#endif
