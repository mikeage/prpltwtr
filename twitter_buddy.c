#include "twitter_buddy.h"

TwitterBuddyData *twitter_buddy_get_buddy_data(PurpleBuddy *b)
{
	if (b->proto_data == NULL)
		b->proto_data = g_new0(TwitterBuddyData, 1);
	return b->proto_data;
}

void twitter_buddy_set_status_data(PurpleAccount *account, const char *src_user, TwitterStatusData *s)
{
	PurpleBuddy *b;
	TwitterBuddyData *buddy_data;
	gboolean status_text_same = FALSE;

	if (!s)
		return;

	if (!s->text)
	{
		twitter_status_data_free(s);
		return;
	}


	b = purple_find_buddy(account, src_user);
	if (!b)
	{
		twitter_status_data_free(s);
		return;
	}

	buddy_data = twitter_buddy_get_buddy_data(b);

	if (buddy_data->status && s->created_at < buddy_data->status->created_at)
	{
		twitter_status_data_free(s);
		return;
	}

	if (buddy_data->status != NULL && s != buddy_data->status)
	{
		status_text_same = (strcmp(buddy_data->status->text, s->text) == 0);
		twitter_status_data_free(buddy_data->status);
	}

	buddy_data->status = s;

	if (!status_text_same)
	{
		purple_prpl_got_user_status(b->account, b->name, TWITTER_STATUS_ONLINE,
				"message", s ? s->text : NULL, NULL);
	}
}

PurpleBuddy *twitter_buddy_new(PurpleAccount *account, const char *screenname, const char *alias)
{
	PurpleGroup *g;
	PurpleBuddy *b = purple_find_buddy(account, screenname);
	const char *group_name;
	if (b != NULL)
	{
		if (b->proto_data == NULL)
			b->proto_data = g_new0(TwitterBuddyData, 1);
		return b;
	}

	group_name = twitter_option_buddy_group(account);
	g = purple_find_group(group_name);
	if (g == NULL)
		g = purple_group_new(group_name);
	b = purple_buddy_new(account, screenname, alias);
	purple_blist_add_buddy(b, NULL, g, NULL);
	b->proto_data = g_new0(TwitterBuddyData, 1);
	return b;
}

void twitter_buddy_set_user_data(PurpleAccount *account, TwitterUserData *u, gboolean add_missing_buddy)
{
	PurpleBuddy *b;
	TwitterBuddyData *buddy_data;
	if (!u || !account)
		return;

	if (!strcmp(u->screen_name, account->username))
	{
		twitter_user_data_free(u);
		return;
	}

	b = purple_find_buddy(account, u->screen_name);
	if (!b && add_missing_buddy)
	{
		/* set alias as screenname (name) */
		gchar *alias = g_strdup_printf ("%s | %s", u->name, u->screen_name);
		b = twitter_buddy_new(account, u->screen_name, alias);
		g_free (alias);
	}

	if (!b)
	{
		twitter_user_data_free(u);
		return;
	}

	buddy_data = twitter_buddy_get_buddy_data(b);

	if (buddy_data == NULL)
		return;
	if (buddy_data->user != NULL && u != buddy_data->user)
		twitter_user_data_free(buddy_data->user);
	buddy_data->user = u;
	twitter_buddy_update_icon(b);
}

void twitter_buddy_update_icon_cb(PurpleUtilFetchUrlData *url_data, gpointer user_data, const gchar *url_text, gsize len, const gchar *error_message)
{
	PurpleBuddy *buddy = user_data;
	TwitterBuddyData *buddy_data = buddy->proto_data;
	if (buddy_data != NULL && buddy_data->user != NULL)
	{
		purple_buddy_icons_set_for_user(buddy->account, buddy->name,
				g_memdup(url_text, len), len, buddy_data->user->profile_image_url);
	}
}

void twitter_buddy_update_icon(PurpleBuddy *buddy)
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
