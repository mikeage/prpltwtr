#ifndef _PT_ENDPOINT_REPLY_H_
#define _PT_ENDPOINT_REPLY_H_

#include "defines.h"

#include "pt_endpoint_im.h"
#include "pt_connection.h"

PtEndpointImSettings *pt_endpoint_reply_get_settings(void);
PurpleConversation *pt_endpoint_reply_conversation_new(PtEndpointIm * im, const gchar * buddy_name, long long reply_id, gboolean force);

#endif
