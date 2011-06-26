#ifndef _TWITTER_ENDPOINT_TIMELINE_H_
#define _TWITTER_ENDPOINT_TIMELINE_H_

#include "defaults.h"

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

#include "prpltwtr_endpoint_chat.h"
#include "prpltwtr_api.h"
#include "prpltwtr_prefs.h"
#include "prpltwtr_xml.h"
#include "prpltwtr_buddy.h"
#include "prpltwtr_conn.h"
#include "prpltwtr_util.h"                       //TODO remove

typedef struct {
    guint           timeline_id;
} TwitterTimelineTimeoutContext;

TwitterEndpointChatSettings *twitter_endpoint_timeline_get_settings(void);

#endif
