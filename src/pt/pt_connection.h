#ifndef _PT_CONNECTION_H_
#define _PT_CONNECTION_H_

#include <glib.h>
#include "pt_requestor.h"
#include "pt_endpoint_im.h"

typedef struct {
    PtRequestor *requestor;

    //long long       failed_get_replies_count;

    //guint           get_friends_timer;
    //guint           update_presence_timer;

    //long long       last_home_timeline_id;

    /* a table of TwitterEndpointChat
     * where the key will be the chat name
     * Alternatively, we only really need chat contexts when
     * we have a blist node with auto_open = TRUE or a chat
     * already open. So we could pass the context between the two
     * but that would be much more annoying to write/maintain */
	GHashTable     *chat_contexts;

    /* key: gchar *screen_name,
     * value: gchar *reply_id (then converted to long long)
     * Store the id of last reply sent from any user to @me
     * This is used as in_reply_to_status_id
     * when @me sends a tweet to others */
    //GHashTable     *user_reply_id_table;

    /* key: gchar *screen_name
     * value: TwitterConvIcon
     * Store purple buddy icons for nonbuddies (for conversations)
     */
    //GHashTable     *icons;

    gboolean        requesting;
    PtEndpointIm *endpoint_ims[PT_IM_TYPE_UNKNOWN];

    gchar          *oauth_token;
    gchar          *oauth_token_secret;

    //TwitterMbPrefs *mb_prefs;

    //int             chat_id;
} PtConnectionData;

void            pt_connection_foreach_endpoint_im(PtConnectionData * conn_data, void (*cb) (PtConnectionData * conn_data, PtEndpointIm * im, gpointer data), gpointer data);
PtEndpointIm *pt_connection_get_endpoint_im(PtConnectionData * conn_data, PtImType type);
void            pt_connection_set_endpoint_im(PtConnectionData * conn_data, PtImType type, PtEndpointIm * endpoint);


#endif
