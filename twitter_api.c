/**
 * TODO: legal stuff
 *
 * purple
 *
 * Purple is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#include <stdarg.h>
#include <string.h>
#include <time.h>

#include <glib.h>

/* If you're using this as the basis of a prpl that will be distributed
 * separately from libpurple, remove the internal.h include below and replace
 * it with code to include your own config.h or similar.  If you're going to
 * provide for translation, you'll also need to setup the gettext macros. */
#include "config.h"

#include "account.h"
#include "accountopt.h"
#include "blist.h"
#include "cmds.h"
#include "conversation.h"
#include "connection.h"
#include "debug.h"
#include "notify.h"
#include "privacy.h"
#include "prpl.h"
#include "roomlist.h"
#include "status.h"
#include "util.h"
#include "version.h"
#include "cipher.h"
#include "request.h"
#include "twitter_request.h"
#include "twitter_api.h"


void twitter_api_get_rate_limit_status(PurpleAccount *account,
		TwitterSendRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data)
{
	twitter_send_request(account, FALSE, 
			"https://twitter.com/account/rate_limit_status.xml", NULL,
			success_func, error_func, data);
}
void twitter_api_get_friends(PurpleAccount *account,
		TwitterSendRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data)
{
	twitter_send_request(account, FALSE,
			"https://twitter.com/statuses/friends.xml", NULL,
			success_func, error_func, NULL);
}
void twitter_api_get_replies(PurpleAccount *account,
		unsigned int since_id,
		TwitterSendRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data)
{
	int count = 20;
	//why strdup?
	char *query = since_id ?
		g_strdup_printf("since_id=%d", since_id) :
		g_strdup("");

	twitter_send_request_multipage(account,
			"https://twitter.com/statuses/replies.xml", query,
			success_func, error_func,
			count, NULL);
	g_free(query);
}

void twitter_api_set_status(PurpleAccount *acct, 
		const char *msg,
		TwitterSendRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data)
{
	if (msg != NULL && strcmp("", msg))
	{
		char *query = g_strdup_printf("status=%s", purple_url_encode(msg));
		twitter_send_request(acct, TRUE,
				"https://twitter.com/statuses/update.xml", query,
				success_func, NULL, data);
		g_free(query);
	} else {
		//SEND error?
	}
}
