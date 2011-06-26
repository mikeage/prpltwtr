#ifndef _TWITTER_ENDPOINT_REPLY_H_
#define _TWITTER_ENDPOINT_REPLY_H_

#include "defaults.h"

#include "prpltwtr_endpoint_im.h"
#include "prpltwtr_conn.h"
#include "prpltwtr_buddy.h"

TwitterEndpointImSettings *twitter_endpoint_reply_get_settings(void);
PurpleConversation *twitter_endpoint_reply_conversation_new(TwitterEndpointIm * im, const gchar * buddy_name, long long reply_id, gboolean force);

#endif
