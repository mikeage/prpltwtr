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
//#include "internal.h"

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


#define TWITTER_PROTOCOL_ID "prpl-twitter"
static PurplePlugin *_twitter_protocol = NULL;

#define TWITTER_STATUS_ONLINE   "online"

#define MAX_TWEET_LENGTH 140

typedef struct _TwitterUserData TwitterUserData;
typedef struct _TwitterStatusData TwitterStatusData;

struct _TwitterUserData
{
	PurpleAccount *account;
	int id;
	char *name;
	char *screen_name;
	char *profile_image_url;
	char *description;
};

struct _TwitterStatusData
{
	char *text;
	int id;
	time_t created_at;
};

typedef struct
{
	TwitterStatusData *status;
	TwitterUserData *user;
} TwitterBuddyData;

typedef struct
{
	guint timer;
} TwitterConnectionData;

void purple_account_set_int(PurpleAccount *account, const char *name, int value);

static int twitter_account_get_last_status_id(PurpleAccount *account)
{
	return purple_account_get_int(account, "twitter_last_status_id", 0);
}
static void twitter_account_set_last_status_id(PurpleAccount *account, int status_id)
{
	purple_account_set_int(account, "twitter_last_status_id", status_id);
}

static char *_(char *txt)
{
	return txt;
}
static char *N_(char *txt)
{
	return txt;
}

//borrowed almost exactly from msn
static time_t twitter_status_parse_timestamp(const char *timestamp)
{
	//Sat Mar 07 18:12:10 +0000 2009
	char month_str[4], tz_str[6];
	char day_name[4];
	char *tz_ptr = tz_str;
	static const char *months[] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec", NULL
	};
	time_t tval = 0;
	struct tm t;
	memset(&t, 0, sizeof(t));

	time(&tval);
	localtime_r(&tval, &t);

	if (sscanf(timestamp, "%03s %03s %02d %02d:%02d:%02d %05s %04d",
				day_name, month_str, &t.tm_mday, 
				&t.tm_hour, &t.tm_min, &t.tm_sec,
				tz_str, &t.tm_year) == 8) {
		gboolean offset_positive = TRUE;
		int tzhrs;
		int tzmins;

		for (t.tm_mon = 0;
				months[t.tm_mon] != NULL &&
				strcmp(months[t.tm_mon], month_str) != 0; t.tm_mon++);
		if (months[t.tm_mon] != NULL) {
			if (*tz_str == '-') {
				offset_positive = FALSE;
				tz_ptr++;
			} else if (*tz_str == '+') {
				tz_ptr++;
			}

			if (sscanf(tz_ptr, "%02d%02d", &tzhrs, &tzmins) == 2) {
				time_t tzoff = tzhrs * 60 * 60 + tzmins * 60;
#ifdef _WIN32
				long sys_tzoff;
#endif

				if (offset_positive)
					tzoff *= -1;

				t.tm_year -= 1900;

#ifdef _WIN32
				if ((sys_tzoff = wpurple_get_tz_offset()) != -1)
					tzoff += sys_tzoff;
#else
#ifdef HAVE_TM_GMTOFF
				tzoff += t.tm_gmtoff;
#else
#	ifdef HAVE_TIMEZONE
				tzset();    /* making sure */
				tzoff -= timezone;
#	endif
#endif
#endif /* _WIN32 */

				return mktime(&t) + tzoff;
			}
		}
	}

	purple_debug_info("msn", "Can't parse timestamp %s\n", timestamp);
	return tval;
}

static char *xmlnode_get_child_data(const xmlnode *node, const char *name)
{
	xmlnode *child = xmlnode_get_child(node, name);
	if (!child)
		return NULL;
	return xmlnode_get_data_unescaped(child);
}


static void twitter_user_data_free(TwitterUserData *user_data)
{
	if (!user_data)
		return;
	if (user_data->name)
		g_free(user_data->name);
	if (user_data->screen_name)
		g_free(user_data->screen_name);
	if (user_data->profile_image_url)
		g_free(user_data->profile_image_url);
	if (user_data->description)
		g_free(user_data->description);
	g_free(user_data);
	user_data = NULL;
}

static void twitter_status_data_free(TwitterStatusData *status)
{
	if (status == NULL)
		return;
	if (status->text != NULL)
		g_free(status->text);
	g_free(status);
}

static TwitterBuddyData *twitter_buddy_get_buddy_data(PurpleBuddy *b)
{
	if (b->proto_data == NULL)
		b->proto_data = g_new0(TwitterBuddyData, 1);
	return b->proto_data;
}

static char *twitter_status_text_get_dst_user(const char *text)
{
	static char buf[MAX_TWEET_LENGTH];
	char *space_pos;
	if (text == NULL)
		return NULL;
	if (text[0] != '@')
		return NULL;
	space_pos = strchr(text, ' ');
	if (space_pos == NULL)
		return NULL;
	strncpy(buf, text + 1, space_pos - text - 1);
	buf[space_pos - text - 1] = '\0';
	return buf;
}

static const char *twitter_status_text_get_text(const char *text)
{
	if (text[0] != '@')
	{
		return text;
	} else {
		return strchr(text, ' ') + 1;
	}
}

static void twitter_buddy_set_twitter_status(PurpleBuddy *b, TwitterStatusData *s)
{
	TwitterBuddyData *buddy_data = twitter_buddy_get_buddy_data(b);

	if (buddy_data->status != NULL && (s == NULL || s != buddy_data->status))
	{
		twitter_status_data_free(buddy_data->status);
	}

	buddy_data->status = s;

	purple_prpl_got_user_status(b->account, b->name, "online",
			"message", s ? s->text : NULL, NULL);

}

static void twitter_buddy_update_icon_cb(PurpleUtilFetchUrlData *url_data, gpointer user_data, const gchar *url_text, gsize len, const gchar *error_message)
{
	PurpleBuddy *buddy = user_data;
	TwitterBuddyData *buddy_data = buddy->proto_data;
	if (buddy_data != NULL && buddy_data->user != NULL)
	{
		purple_buddy_icons_set_for_user(buddy->account, buddy->name,
				g_memdup(url_text, len), len, buddy_data->user->profile_image_url);
	}
}
static void twitter_buddy_update_icon(PurpleBuddy *buddy)
{
	const char *previous_url = purple_buddy_icons_get_checksum_for_user(buddy);
	TwitterBuddyData *twitter_buddy_data = buddy->proto_data;
	if (twitter_buddy_data == NULL || twitter_buddy_data->user == NULL)
	{
		return;
	} else {
		const char *url = twitter_buddy_data->user->profile_image_url;
		if (url == NULL)
		{
			purple_buddy_icons_set_for_user(buddy->account, buddy->name, NULL, 0, NULL);
		} else {
			if (!previous_url || !g_str_equal(previous_url, url)) {
				purple_util_fetch_url(url, TRUE, NULL, FALSE, twitter_buddy_update_icon_cb, buddy);
			}
		}
	}
}
static void twitter_buddy_set_user_data(PurpleBuddy *b, TwitterUserData *data)
{
	TwitterBuddyData *buddy_data = twitter_buddy_get_buddy_data(b);
	if (buddy_data == NULL)
		return;
	if (buddy_data->user != NULL && (data != NULL || data != buddy_data->user))
		twitter_user_data_free(buddy_data->user);
	buddy_data->user = data;
	twitter_buddy_update_icon(b);
}

static PurpleBuddy *twitter_buddy_new(PurpleAccount *account, const char *screenname, const char *alias)
{
	PurpleGroup *g;
	PurpleBuddy *b = purple_find_buddy(account, screenname);
	if (b != NULL)
	{
		if (b->proto_data == NULL)
			b->proto_data = g_new0(TwitterBuddyData, 1);
		return b;
	}

	g = purple_find_group("twitter");
	if (g == NULL)
		g = purple_group_new("twitter");
	b = purple_buddy_new(account, screenname, alias);
	purple_blist_add_buddy(b, NULL, g, NULL);
	b->proto_data = g_new0(TwitterBuddyData, 1);
	return b;
}
static void twitter_error_cb(PurpleAccount *account, xmlnode *node, gpointer user_data)
{
	char *error_msg = xmlnode_get_child_data(node, "error");
	if (!strcmp(error_msg, "This method requires authentication."))
	{
		PurpleConnection *gc = purple_account_get_connection(account);
		char *err = _("Invalid password.");
		purple_connection_error_reason(gc, PURPLE_CONNECTION_ERROR_AUTHENTICATION_FAILED, err);

	}
}

static void twitter_user_data_handle_new(PurpleAccount *account, TwitterUserData *u, gboolean add_missing_buddy)
{
	PurpleBuddy *b;
	if (!u)
		return;

	if (!strcmp(u->screen_name, account->username))
	{
		twitter_user_data_free(u);
		return;
	}
	b = purple_find_buddy(account, u->screen_name);
	if (!b)
	{
		if (add_missing_buddy)
		{
			b = twitter_buddy_new(account, u->screen_name, u->name);
		}
		if (!b)
		{
			twitter_user_data_free(u);
			return;
		}
	}

	twitter_buddy_set_user_data(b, u);
}
static void twitter_status_data_handle_new(PurpleAccount *account, char *src_user, TwitterStatusData *s, gboolean update_conv)
{
	PurpleBuddy *b;

	if (!s)
		return;

	if (!s->text)
	{
		twitter_status_data_free(s);
		return;
	}

	if (s->id && s->id > twitter_account_get_last_status_id(account))
	{
		twitter_account_set_last_status_id(account, s->id);
	}


	if (update_conv)
	{
		char *dst_user = twitter_status_text_get_dst_user(s->text);
		if (dst_user && !strcmp(dst_user, account->username))
		{
			//Account received an im
			PurpleMessageFlags receive_flags = PURPLE_MESSAGE_RECV;
			serv_got_im(purple_account_get_connection(account), src_user,
					twitter_status_text_get_text(s->text),
					receive_flags, s->created_at);
		}


		/*
		   if (!strcmp(src_user, account->username))
		   {
		//Account sent an im
		PurpleConversation *conv = purple_find_conversation_with_account(
		PURPLE_CONV_TYPE_IM, dst_user,
		account);
		PurpleConvIm *conv_im;
		if (!conv)
		{
		conv = purple_conversation_new(PURPLE_CONV_TYPE_IM, account, dst_user);
		}
		if (conv && (conv_im = purple_conversation_get_im_data(conv)) != NULL)
		{
		purple_conversation_get_im_data(conv);
		if (conv_im)
		purple_conv_im_write(conv_im, account->username,
		twitter_status_text_get_text(s->text), PURPLE_MESSAGE_SEND,
		s->created_at);
		}
		}
		*/

	}

	b = purple_find_buddy(account, src_user);
	if (b)
		twitter_buddy_set_twitter_status(b, s);
	else
		twitter_status_data_free(s);
}

/*static PurpleBuddy *twitter_buddy_new_from_twitter_data(PurpleAccount *account, TwitterUserData *twitter_user_data)
  {
  PurpleBuddy *b = twitter_buddy_new(account, twitter_user_data->screen_name, twitter_user_data->name);
  b->proto_data = twitter_user_data;
  twitter_buddy_update_icon(b);

  return b;
  }*/


static TwitterUserData *twitter_user_node_parse(xmlnode *node);
static TwitterStatusData *twitter_status_node_parse(xmlnode *status_node)
{
	TwitterStatusData *status;
	char *data;

	if (status_node == NULL)
		return NULL;

	status = g_new0(TwitterStatusData, 1);
	status->text = xmlnode_get_child_data(status_node, "text");

	if ((data = xmlnode_get_child_data(status_node, "created_at")))
	{
		time_t created_at = twitter_status_parse_timestamp(data);
		status->created_at = created_at ? created_at : time(NULL);
		g_free(data);
	}

	if ((data = xmlnode_get_child_data(status_node, "id")))
	{
		status->id =atoi(data);
		g_free(data);
	}

	return status;

}

static TwitterUserData *twitter_user_node_parse(xmlnode *user_node)
{
	TwitterUserData *user;

	if (user_node == NULL)
		return NULL;

	user = g_new0(TwitterUserData, 1);
	user->screen_name = xmlnode_get_child_data(user_node, "screen_name");

	if (!user->screen_name)
	{
		g_free(user);
		return NULL;
	}

	user->name = xmlnode_get_child_data(user_node, "name");
	user->profile_image_url = xmlnode_get_child_data(user_node, "profile_image_url");
	user->description = xmlnode_get_child_data(user_node, "description");

	return user;
}
static GList *twitter_users_node_parse(xmlnode *users_node)
{
	GList *users = NULL;
	xmlnode *user_node;
	for (user_node = users_node->child; user_node; user_node = user_node->next)
	{
		if (user_node->name && !strcmp(user_node->name, "user"))
		{
			TwitterBuddyData *data = g_new0(TwitterBuddyData, 1);

			xmlnode *status_node = xmlnode_get_child(user_node, "status");

			data->user = twitter_user_node_parse(user_node);
			data->status = twitter_status_node_parse(status_node);

			users = g_list_append(users, data);
		}
	}
	return users;
}
static GList *twitter_statuses_node_parse(xmlnode *statuses_node)
{
	GList *statuses = NULL;
	xmlnode *status_node;
	for (status_node = statuses_node->child; status_node; status_node = status_node->next)
	{
		if (status_node->name && !strcmp(status_node->name, "status"))
		{
			TwitterBuddyData *data = g_new0(TwitterBuddyData, 1);
			xmlnode *user_node = xmlnode_get_child(status_node, "user");
			data->user = twitter_user_node_parse(user_node);
			data->status = twitter_status_node_parse(status_node);
			statuses = g_list_prepend(statuses, data);
		}
	}
	return statuses;
}


static void twitter_send_im_cb(PurpleAccount *account, xmlnode *node, gpointer user_data)
{
	//TODO: verify status was sent
	return;
	/*
	   TwitterStatusData *s = twitter_status_node_parse(node);
	   xmlnode *user_node = xmlnode_get_child(node, "user");
	   TwitterUserData *u = twitter_user_node_parse(user_node);

	   if (!s)
	   return;

	   if (!u)
	   {
	   twitter_status_data_free(s);
	   return;
	   }

	   twitter_status_data_handle_new(account, u->screen_name, s, FALSE);
	   twitter_user_data_free(u);
	   */
}

static void twitter_get_friends_cb(PurpleAccount *account, xmlnode *node, gpointer user_data)
{
	//TODO handle multiple pages of data
	GList *users = twitter_users_node_parse(node);
	GList *l;
	for (l = users; l; l = l->next)
	{
		TwitterBuddyData *data = l->data;
		TwitterUserData *user = data->user;
		TwitterStatusData *status = data->status;
		char *screen_name = g_strdup(user->screen_name);
		g_free(data);

		twitter_user_data_handle_new(account, user, TRUE);
		if (status != NULL)
			twitter_status_data_handle_new(account, screen_name, status, FALSE);
		g_free(screen_name);


	}
}

static void twitter_account_set_buddies_online(PurpleAccount *account)
{
	GSList *buddy; GSList *buddies;
	buddies = purple_find_buddies(account, NULL);
	for (buddy = buddies; buddy; buddy = buddy->next)
	{
		purple_prpl_got_user_status(account, ((PurpleBuddy *) buddy->data)->name, "online", NULL);
	}
}


static void twitter_get_friends_custom(PurpleAccount *account,
		TwitterSendRequestFunc success_func,
		TwitterSendRequestFunc error_func)
{
	twitter_send_request(account, FALSE,
			"http://twitter.com/statuses/friends.xml", NULL,
			success_func, error_func, NULL);
}
static void twitter_get_friends(PurpleAccount *account)
{
	twitter_get_friends_custom(account, twitter_get_friends_cb, twitter_error_cb);
}
static void twitter_get_replies_cb(PurpleAccount *account, xmlnode *node, gpointer user_data)
{
	GList *statuses = twitter_statuses_node_parse(node);
	GList *l = statuses;

	for (l = statuses; l; l = l->next)
	{
		TwitterBuddyData *data = l->data;
		TwitterStatusData *status = data->status;
		TwitterUserData *user_data = data->user;
		g_free(data);

		if (!user_data)
		{
			twitter_status_data_free(status);
		} else {
			char *screen_name = g_strdup(user_data->screen_name);
			twitter_user_data_handle_new(account, user_data, FALSE);
			twitter_status_data_handle_new(account, screen_name, status, TRUE);
			g_free(screen_name);
		}
	}
	g_list_free(statuses);
}

static void twitter_get_replies(PurpleAccount *account,
		unsigned int since_id)
{
	int count = 20;
	//why strdup?
	char *query = since_id ?
		g_strdup_printf("since_id=%d", since_id) :
		g_strdup("");

	twitter_send_request_multipage(account,
			"http://twitter.com/statuses/replies.xml", query,
			twitter_get_replies_cb, NULL,
			count, NULL);
	g_free(query);
}

static gboolean twitter_timeout(gpointer data)
{
	PurpleAccount *account = data;
	twitter_get_replies(account, twitter_account_get_last_status_id(account));
	return TRUE;
}

static void twitter_get_friends_verify_connection_cb(PurpleAccount *account, xmlnode *node, gpointer user_data)
{
	PurpleConnection *gc = purple_account_get_connection(account);
	if (purple_connection_get_state(gc) != PURPLE_CONNECTED)
	{

		TwitterConnectionData *twitter = gc->proto_data;
		purple_connection_update_progress(gc, _("Connected"),
				1,   /* which connection step this is */
				2);  /* total number of steps */
		purple_connection_set_state(gc, PURPLE_CONNECTED);

		twitter_account_set_buddies_online(account);
		twitter->timer = purple_timeout_add(1000 * 60, twitter_timeout, account);
	}

	twitter_get_friends_cb(account, node, user_data);
}


static void twitter_get_rate_limit_status_cb(PurpleAccount *account, xmlnode *node, gpointer user_data)
{
	/*
	   <hash>
	   <reset-time-in-seconds type="integer">1236529763</reset-time-in-seconds>
	   <remaining-hits type="integer">100</remaining-hits>
	   <hourly-limit type="integer">100</hourly-limit>
	   <reset-time type="datetime">2009-03-08T16:29:23+00:00</reset-time>
	   </hash>
	   */

	xmlnode *child;
	int remaining_hits = 0;
	int hourly_limit = 0;
	char *message;
	for (child = node->child; child; child = child->next)
	{
		if (child->name)
		{
			if (!strcmp(child->name, "remaining-hits"))
			{
				char *data = xmlnode_get_data_unescaped(child);
				remaining_hits = atoi(data);
				g_free(data);
			}
			else if (!strcmp(child->name, "hourly-limit"))
			{
				char *data = xmlnode_get_data_unescaped(child);
				hourly_limit = atoi(data);
				g_free(data);
			}
		}
	}
	message = g_strdup_printf("%d/%d %s", remaining_hits, hourly_limit, _("Remaining"));
	purple_notify_info(NULL,  /* plugin handle or PurpleConnection */
			_("Rate Limit Status"),
			_("Rate Limit Status"),
			_(message));
	g_free(message);
}
static void twitter_get_rate_limit_status(PurpleAccount *account)
{
	twitter_send_request(account, FALSE, 
			"http://twitter.com/account/rate_limit_status.xml", NULL,
			twitter_get_rate_limit_status_cb, NULL, account);
}

/*
 * prpl functions
 */
static const char *twitter_list_icon(PurpleAccount *acct, PurpleBuddy *buddy)
{
	/* shamelessly steal (er, borrow) the meanwhile protocol icon. it's cute! */
	return "meanwhile";
}

static char *twitter_status_text(PurpleBuddy *buddy) {
	purple_debug_info("twitter", "getting %s's status text for %s\n",
			buddy->name, buddy->account->username);

	if (purple_find_buddy(buddy->account, buddy->name)) {
		PurplePresence *presence = purple_buddy_get_presence(buddy);
		PurpleStatus *status = purple_presence_get_active_status(presence);
		const char *message = status ? purple_status_get_attr_string(status, "message") : NULL;

		if (message && strlen(message) > 0)
			return g_strdup(g_markup_escape_text(message, -1));

	}
	return NULL;
}

static void twitter_tooltip_text(PurpleBuddy *buddy,
		PurpleNotifyUserInfo *info,
		gboolean full) {

	if (PURPLE_BUDDY_IS_ONLINE(buddy))
	{
		PurplePresence *presence = purple_buddy_get_presence(buddy);
		PurpleStatus *status = purple_presence_get_active_status(presence);
		char *msg = twitter_status_text(buddy);
		purple_notify_user_info_add_pair(info, purple_status_get_name(status),
				msg);
		g_free(msg);

		if (full) {
			/*const char *user_info = purple_account_get_user_info(gc->account);
			  if (user_info)
			  purple_notify_user_info_add_pair(info, _("User info"), user_info);*/
		}

		purple_debug_info("twitter", "showing %s tooltip for %s\n",
				(full) ? "full" : "short", buddy->name);
	}
}

/* everyone will be online
 * Future: possibly have offline mode for people who haven't updated in a while
 */
static GList *twitter_status_types(PurpleAccount *acct)
{
	GList *types = NULL;
	PurpleStatusType *type;

	type = purple_status_type_new(PURPLE_STATUS_AVAILABLE, TWITTER_STATUS_ONLINE,
			TWITTER_STATUS_ONLINE, TRUE);
	purple_status_type_add_attr(type, "message", _("Online"),
			purple_value_new(PURPLE_TYPE_STRING));
	types = g_list_prepend(types, type);

	return types;
}

/*
   static void twitter_get_friends_timeline_cb(PurpleAccount *account, xmlnode *node, gpointer user_data)
   {
   GList *statuses = twitter_statuses_node_parse(node);

//TODO

g_list_free(statuses);
}

static void twitter_get_friends_timeline(PurpleAccount *account,
unsigned int since_id)
{
int count = 200;

//why strdup?
char *query = since_id ? 
g_strdup_printf("since_id=%d&count=%d", since_id, count) :
g_strdup_printf("count=%d", count);


twitter_send_request_multipage(account,
"http://twitter.com/statuses/friends_timeline.xml", query,
twitter_get_friends_timeline_cb, NULL,
count);
g_free(query);

}*/



/* 
 * UI callbacks
 */
static void twitter_action_get_user_info(PurplePluginAction *action)
{
	PurpleConnection *gc = (PurpleConnection *)action->context;
	PurpleAccount *acct = purple_connection_get_account(gc);
	twitter_get_friends(acct);
}

	static void
twitter_request_id_ok(PurpleConnection *gc, PurpleRequestFields *fields)
{
	PurpleAccount *acct = purple_connection_get_account(gc);
	int id = purple_request_fields_get_integer(fields, "id");
	twitter_get_replies(acct, id);
}
static void twitter_action_get_timeline(PurplePluginAction *action)
{
	PurpleRequestFields *request;
	PurpleRequestFieldGroup *group;
	PurpleRequestField *field;
	PurpleConnection *gc = (PurpleConnection *)action->context;

	group = purple_request_field_group_new(NULL);

	field = purple_request_field_int_new("id", _("Minutes"), 0);
	purple_request_field_group_add_field(group, field);

	request = purple_request_fields_new();
	purple_request_fields_add_group(request, group);

	purple_request_fields(action->plugin,
			N_("Timeline"),
			_("Set last time"),
			NULL,
			request,
			_("_Set"), G_CALLBACK(twitter_request_id_ok),
			_("_Cancel"), NULL,
			purple_connection_get_account(gc), NULL, NULL,
			gc);
}
static void twitter_action_get_rate_limit_status(PurplePluginAction *action)
{
	PurpleConnection *gc = (PurpleConnection *)action->context;
	PurpleAccount *acct = purple_connection_get_account(gc);
	twitter_get_rate_limit_status(acct);
}

/* this is set to the actions member of the PurplePluginInfo struct at the
 * bottom.
 */
static GList *twitter_actions(PurplePlugin *plugin, gpointer context)
{
	GList *l = NULL;
	PurplePluginAction *action;

	action = purple_plugin_action_new("Retrieve users", twitter_action_get_user_info);
	l = g_list_append(l, action);

	action = purple_plugin_action_new("Retrieve replies", twitter_action_get_timeline);
	l = g_list_append(l, action);

	action = purple_plugin_action_new("Rate Limit Status", twitter_action_get_rate_limit_status);
	l = g_list_append(l, action);

	return l;
}



static void twitter_verify_connection(PurpleAccount *acct)
{
	twitter_get_friends_custom(acct, twitter_get_friends_verify_connection_cb, twitter_error_cb);
}
static void twitter_login(PurpleAccount *acct)
{
	PurpleConnection *gc = purple_account_get_connection(acct);
	TwitterConnectionData *twitter = g_new0(TwitterConnectionData, 1);
	gc->proto_data = twitter;

	purple_debug_info("twitter", "logging in %s\n", acct->username);

	/* purple wants a minimum of 2 steps */
	purple_connection_update_progress(gc, _("Connecting"),
			0,   /* which connection step this is */
			2);  /* total number of steps */

	twitter_verify_connection(acct);



}

static void twitter_close(PurpleConnection *gc)
{
	/* notify other twitter accounts */
	TwitterConnectionData *twitter = gc->proto_data;

	if (twitter->timer)
		purple_timeout_remove(twitter->timer);

	g_free(twitter);
}

static int twitter_send_im(PurpleConnection *gc, const char *who,
		const char *message, PurpleMessageFlags flags)
{
	if (strlen(who) + strlen(message) + 2 > MAX_TWEET_LENGTH)
	{
		purple_conv_present_error(who, purple_connection_get_account(gc), "Message is too long");
		return 0;
	}
	else
	{
		char *status = g_strdup_printf("@%s %s", who, message);
		char *query = g_strdup_printf("status=%s", purple_url_encode(status));
		twitter_send_request(purple_connection_get_account(gc), TRUE,
				"http://twitter.com/statuses/update.xml", query,
				twitter_send_im_cb, NULL, NULL);
		g_free(status);
		g_free(query);
		return 1;
	}
	//  const char *from_username = gc->account->username;
	//  PurpleMessageFlags receive_flags = ((flags & ~PURPLE_MESSAGE_SEND)
	//				      | PURPLE_MESSAGE_RECV);
	//  PurpleAccount *to_acct = purple_accounts_find(who, TWITTER_PROTOCOL_ID);
	//  PurpleConnection *to;
	//
	//  purple_debug_info("twitter", "sending message from %s to %s: %s\n",
	//		    from_username, who, message);
	//
	//  /* is the sender blocked by the recipient's privacy settings? */
	//  if (to_acct && !purple_privacy_check(to_acct, gc->account->username)) {
	//    char *msg = g_strdup_printf(
	//      _("Your message was blocked by %s's privacy settings."), who);
	//    purple_debug_info("twitter",
	//		      "discarding; %s is blocked by %s's privacy settings\n",
	//		      from_username, who);
	//    purple_conv_present_error(who, gc->account, msg);
	//    g_free(msg);
	//    return 0;
	//  }
	//
	//  /* is the recipient online? */
	//  to = get_twitter_gc(who);
	//  if (to) {  /* yes, send */
	//  PurpleMessageFlags receive_flags = ((flags & ~PURPLE_MESSAGE_SEND)
	//				      | PURPLE_MESSAGE_RECV);
	//    serv_got_im(to, from_username, message, receive_flags, time(NULL));
	//
	//  } else {  /* nope, store as an offline message */
	//    GOfflineMessage *offline_message;
	//    GList *messages;
	//
	//    purple_debug_info("twitter",
	//		      "%s is offline, sending as offline message\n", who);
	//    offline_message = g_new0(GOfflineMessage, 1);
	//    offline_message->from = g_strdup(from_username);
	//    offline_message->message = g_strdup(message);
	//    offline_message->mtime = time(NULL);
	//    offline_message->flags = receive_flags;
	//
	//    messages = g_hash_table_lookup(goffline_messages, who);
	//    messages = g_list_append(messages, offline_message);
	//    g_hash_table_insert(goffline_messages, g_strdup(who), messages);
	//  }
	//
	//   return 1;
}

static void twitter_set_info(PurpleConnection *gc, const char *info) {
	purple_debug_info("twitter", "setting %s's user info to %s\n",
			gc->account->username, info);
}

static void twitter_get_info(PurpleConnection *gc, const char *username) {
	//TODO: error check
	PurpleBuddy *b = purple_find_buddy(purple_connection_get_account(gc), username);
	TwitterBuddyData *data = twitter_buddy_get_buddy_data(b);
	TwitterUserData *user_data = data->user;

	PurpleNotifyUserInfo *info = purple_notify_user_info_new();

	//body = _("No user info.");
	if (user_data)
	{
		purple_notify_user_info_add_pair(info, "Description", user_data->description);
	}

	/* show a buddy's user info in a nice dialog box */
	purple_notify_userinfo(gc,	/* connection the buddy info came through */
			username,  /* buddy's username */
			info,      /* body */
			NULL,      /* callback called when dialog closed */
			NULL);     /* userdata for callback */
}

static void twitter_send_update_status(PurpleAccount *acct, const char *msg, TwitterSendRequestFunc success_func, gpointer data)
{
	if (msg != NULL && strcmp("", msg))
	{
		char *query = g_strdup_printf("status=%s", purple_url_encode(msg));
		twitter_send_request(acct, TRUE,
				"http://twitter.com/statuses/update.xml", query,
				success_func, NULL, data);
		g_free(query);
	}
}
static void twitter_set_status(PurpleAccount *acct, PurpleStatus *status) {
	const char *msg = purple_status_get_attr_string(status, "message");
	purple_debug_info("twitter", "setting %s's status to %s: %s\n",
			acct->username, purple_status_get_name(status), msg);

	twitter_send_update_status(acct, msg, NULL, NULL);
}

static void twitter_add_buddy(PurpleConnection *gc, PurpleBuddy *buddy,
		PurpleGroup *group)
{
	//TODO
}

static void twitter_add_buddies(PurpleConnection *gc, GList *buddies,
		GList *groups) {
	GList *buddy = buddies;
	GList *group = groups;

	purple_debug_info("twitter", "adding multiple buddies\n");

	while (buddy && group) {
		twitter_add_buddy(gc, (PurpleBuddy *)buddy->data, (PurpleGroup *)group->data);
		buddy = g_list_next(buddy);
		group = g_list_next(group);
	}
}

static void twitter_remove_buddy(PurpleConnection *gc, PurpleBuddy *buddy,
		PurpleGroup *group)
{
	TwitterBuddyData *twitter_buddy_data = buddy->proto_data;

	purple_debug_info("twitter", "removing %s from %s's buddy list\n",
			buddy->name, gc->account->username);

	if (!twitter_buddy_data)
		return;
	twitter_user_data_free(twitter_buddy_data->user);
	twitter_status_data_free(twitter_buddy_data->status);
	g_free(twitter_buddy_data);
	buddy->proto_data = NULL;
}

static void twitter_remove_buddies(PurpleConnection *gc, GList *buddies,
		GList *groups) {
	GList *buddy = buddies;
	GList *group = groups;

	purple_debug_info("twitter", "removing multiple buddies\n");

	while (buddy && group) {
		twitter_remove_buddy(gc, (PurpleBuddy *)buddy->data,
				(PurpleGroup *)group->data);
		buddy = g_list_next(buddy);
		group = g_list_next(group);
	}
}

static void twitter_get_cb_info(PurpleConnection *gc, int id, const char *who) {
	PurpleConversation *conv = purple_find_chat(gc, id);
	purple_debug_info("twitter",
			"retrieving %s's info for %s in chat room %s\n", who,
			gc->account->username, conv->name);

	twitter_get_info(gc, who);
}

/* normalize a username (e.g. remove whitespace, add default domain, etc.)
 * for twitter, this is a noop.
 */
static const char *twitter_normalize(const PurpleAccount *acct,
		const char *input) {
	purple_debug_info("Twitter", "twitter_normalize"); //?
	return NULL;
}

static void twitter_set_buddy_icon(PurpleConnection *gc,
		PurpleStoredImage *img) {
	purple_debug_info("twitter", "setting %s's buddy icon to %s\n",
			gc->account->username, purple_imgstore_get_filename(img));
}


/*
 * prpl stuff. see prpl.h for more information.
 */

static PurplePluginProtocolInfo prpl_info =
{
	OPT_PROTO_CHAT_TOPIC,  /* options */
	NULL,	       /* user_splits, initialized in twitter_init() */
	NULL,	       /* protocol_options, initialized in twitter_init() */
	{   /* icon_spec, a PurpleBuddyIconSpec */
		"png,jpg,gif",		   /* format */
		0,			       /* min_width */
		0,			       /* min_height */
		48,			     /* max_width */
		48,			     /* max_height */
		10000,			   /* max_filesize */
		PURPLE_ICON_SCALE_DISPLAY,       /* scale_rules */
	},
	twitter_list_icon,		  /* list_icon */ //TODO
	NULL, //twitter_list_emblem,		/* list_emblem */
	twitter_status_text,		/* status_text */
	twitter_tooltip_text,	       /* tooltip_text */
	twitter_status_types,	       /* status_types */
	NULL, //twitter_blist_node_menu,	    /* blist_node_menu */
	NULL,		  /* chat_info */
	NULL,	 /* chat_info_defaults */
	twitter_login,		      /* login */
	twitter_close,		      /* close */
	twitter_send_im,		    /* send_im */
	twitter_set_info,		   /* set_info */
	NULL, //twitter_send_typing,		/* send_typing */
	twitter_get_info,		   /* get_info */
	twitter_set_status,		 /* set_status */
	NULL,		   /* set_idle */
	NULL,//TODO?	      /* change_passwd */
	twitter_add_buddy,//TODO		  /* add_buddy */
	twitter_add_buddies,//TODO		/* add_buddies */
	twitter_remove_buddy,//TODO	       /* remove_buddy */
	twitter_remove_buddies,//TODO	     /* remove_buddies */
	NULL,//TODO?		 /* add_permit */
	NULL,//TODO?		   /* add_deny */
	NULL,//TODO?		 /* rem_permit */
	NULL,//TODO?		   /* rem_deny */
	NULL,//TODO?	    /* set_permit_deny */
	NULL,		  /* join_chat */
	NULL,		/* reject_chat */
	NULL,	      /* get_chat_name */
	NULL,		/* chat_invite */
	NULL,		 /* chat_leave */
	NULL,//twitter_chat_whisper,	       /* chat_whisper */
	NULL,		  /* chat_send */
	NULL,//TODO?				/* keepalive */
	NULL,	      /* register_user */
	twitter_get_cb_info,		/* get_cb_info */
	NULL,				/* get_cb_away */
	NULL,//TODO?		/* alias_buddy */
	NULL,		/* group_buddy */
	NULL,	       /* rename_group */
	NULL,				/* buddy_free */
	NULL,	       /* convo_closed */
	twitter_normalize,		  /* normalize */
	twitter_set_buddy_icon,	     /* set_buddy_icon */
	NULL,	       /* remove_group */
	NULL,//TODO?				/* get_cb_real_name */
	NULL,	     /* set_chat_topic */
	NULL,				/* find_blist_chat */
	NULL,	  /* roomlist_get_list */
	NULL,	    /* roomlist_cancel */
	NULL,   /* roomlist_expand_category */
	NULL,	   /* can_receive_file */
	NULL,				/* send_file */
	NULL,				/* new_xfer */
	NULL,	    /* offline_message */
	NULL,				/* whiteboard_prpl_ops */
	NULL,				/* send_raw */
	NULL,				/* roomlist_room_serialize */
	NULL,				/* padding... */
	NULL,
	NULL,
	sizeof(PurplePluginProtocolInfo),    /* struct_size */
	NULL
};

static void twitter_init(PurplePlugin *plugin)
{
	purple_debug_info("twitter", "starting up\n");

	_twitter_protocol = plugin;
}

static void twitter_destroy(PurplePlugin *plugin) {
	purple_debug_info("twitter", "shutting down\n");
}


static PurplePluginInfo info =
{
	PURPLE_PLUGIN_MAGIC,				     /* magic */
	PURPLE_MAJOR_VERSION,				    /* major_version */
	PURPLE_MINOR_VERSION,				    /* minor_version */
	PURPLE_PLUGIN_PROTOCOL,				  /* type */
	NULL,						    /* ui_requirement */
	0,						       /* flags */
	NULL,						    /* dependencies */
	PURPLE_PRIORITY_DEFAULT,				 /* priority */
	TWITTER_PROTOCOL_ID,					     /* id */
	"Twitter Protocol",					      /* name */
	"0.1",						   /* version */
	"Twitter Protocol Plugin",				  /* summary */
	"Twitter Protocol Plugin",				  /* description */
	"neaveru <neaveru@gmail.com>",		     /* author */
	"http://code.google.com/p/libpurple-twitter-protocol/",  /* homepage */
	NULL,						    /* load */
	NULL,						    /* unload */
	twitter_destroy,					/* destroy */
	NULL,						    /* ui_info */
	&prpl_info,					      /* extra_info */
	NULL,						    /* prefs_info */
	twitter_actions,					/* actions */
	NULL,						    /* padding... */
	NULL,
	NULL,
	NULL,
};

PURPLE_INIT_PLUGIN(null, twitter_init, info);
