#include "twitter_buddy.h"

//TODO this should be TwitterBuddy
TwitterUserTweet *twitter_buddy_get_buddy_data(PurpleBuddy *b)
{
	if (b->proto_data == NULL)
	{
		TwitterUserTweet *twitter_buddy = twitter_user_tweet_new(b->name, NULL, NULL);
		b->proto_data = twitter_buddy;
	}
	return b->proto_data;
}

static time_t twitter_account_get_online_cutoff(PurpleAccount *account)
{
	int cutoff_hours = twitter_option_cutoff_time(account);
	if (cutoff_hours == 0)
		return 0;
	else
		return time(NULL) - 60 * 60 * cutoff_hours;
}

static void twitter_buddy_change_state(PurpleBuddy *buddy, gboolean online, const gchar *message)
{
	if (online == PURPLE_BUDDY_IS_ONLINE(buddy))
		return;
	purple_prpl_got_user_status(purple_buddy_get_account(buddy), buddy->name,
			online ? TWITTER_STATUS_ONLINE : TWITTER_STATUS_OFFLINE,
			"message", message,
			NULL);
}

void twitter_buddy_touch_state_with_cutoff(PurpleBuddy *buddy, time_t cutoff)
{
	PurpleAccount *account = purple_buddy_get_account(buddy);
	TwitterUserTweet *user_tweet = twitter_buddy_get_buddy_data(buddy);
	TwitterTweet *tweet = user_tweet ? user_tweet->status : NULL;
	gchar *tweet_message = tweet ? tweet->text : NULL;

	//Yes, I know this could be 'shorter'. But this is (somewhat) clearer
	if (!cutoff) //Always set buddies to online
	{
		if (twitter_option_get_following(account) && !user_tweet)
		{
			//This user was added to the buddy list, but we aren't following them
			//set the user offline
			twitter_buddy_change_state(buddy, FALSE, tweet_message);
		} else {
			//Either get_following is true and the user_tweet has been seen, or
			//get_following is false, so we just set them to online
			twitter_buddy_change_state(buddy, TRUE, tweet_message);
		}
	} else {
		if (!tweet || tweet->created_at < cutoff)
		{
			//No tweet or the tweet was created before the cutoff, set offline
			twitter_buddy_change_state(buddy, FALSE, tweet_message);
		} else {
			//Tweet after the cutoff, set to online
			twitter_buddy_change_state(buddy, TRUE, tweet_message);
		}
	}
}

void twitter_buddy_touch_state(PurpleBuddy *buddy)
{
	PurpleAccount *account = purple_buddy_get_account(buddy);
	time_t cutoff = twitter_account_get_online_cutoff(account);
	twitter_buddy_touch_state_with_cutoff(buddy, cutoff);
}

void twitter_buddy_touch_state_all(PurpleAccount *account)
{
	GSList *buddies;
	GSList *b_node;
	time_t cutoff = twitter_account_get_online_cutoff(account);

	if (!cutoff)
		return;

	buddies = purple_find_buddies(account, NULL);

	for (b_node = buddies; b_node; b_node = g_slist_next(b_node))
	{
		twitter_buddy_touch_state_with_cutoff((PurpleBuddy *) b_node->data, cutoff);
	}

	g_slist_free(buddies);
}

void twitter_buddy_set_status_data(PurpleAccount *account, const char *src_user, TwitterTweet *s)
{
	PurpleBuddy *b;
	TwitterUserTweet *buddy_data;
	gboolean status_text_same = FALSE;
	time_t cutoff = twitter_account_get_online_cutoff(account);

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
		purple_prpl_got_user_status(b->account, b->name, 
				cutoff && s && s->created_at < cutoff ? TWITTER_STATUS_OFFLINE : TWITTER_STATUS_ONLINE,
				"message", s ? s->text : NULL, NULL);
	}
}

PurpleBuddy *twitter_buddy_new(PurpleAccount *account, const char *screenname, const char *alias)
{
	PurpleGroup *g;
	PurpleBuddy *b = purple_find_buddy(account, screenname);
	TwitterUserTweet *twitter_buddy;
	const char *group_name;
	if (b != NULL)
	{
		if (b->proto_data == NULL)
		{
			b->proto_data = twitter_user_tweet_new(screenname, NULL, NULL);
		}
		return b;
	}

	group_name = twitter_option_buddy_group(account);
	g = purple_find_group(group_name);
	if (g == NULL)
		g = purple_group_new(group_name);
	b = purple_buddy_new(account, screenname, alias);
	purple_blist_add_buddy(b, NULL, g, NULL);
	twitter_buddy = twitter_user_tweet_new(screenname, NULL, NULL);
	b->proto_data = twitter_buddy;
	return b;
}

void twitter_buddy_set_user_data(PurpleAccount *account, TwitterUserData *u, gboolean add_missing_buddy)
{
	PurpleBuddy *b;
	TwitterUserTweet *buddy_data;
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
	TwitterUserTweet *buddy_data = buddy->proto_data;
	if (buddy_data != NULL && buddy_data->user != NULL)
	{
		purple_buddy_icons_set_for_user(buddy->account, buddy->name,
				g_memdup(url_text, len), len, buddy_data->user->profile_image_url);
	}
}

void twitter_buddy_update_icon(PurpleBuddy *buddy)
{
	const char *previous_url = purple_buddy_icons_get_checksum_for_user(buddy);
	TwitterUserTweet *twitter_buddy_data = buddy->proto_data;
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
