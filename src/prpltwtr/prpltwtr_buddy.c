#include "prpltwtr_buddy.h"
#include "prpltwtr_util.h"

//TODO this should be TwitterBuddy
TwitterUserTweet *twitter_buddy_get_buddy_data(PurpleBuddy * b)
{
    if (b->proto_data == NULL) {
        TwitterUserTweet *twitter_buddy = twitter_user_tweet_new(b->name, NULL, NULL, NULL);
        b->proto_data = twitter_buddy;
    }
    return b->proto_data;
}

static time_t twitter_account_get_online_cutoff(PurpleAccount * account)
{
    int             cutoff_hours = twitter_option_cutoff_time(account);
    if (cutoff_hours == 0)
        return 0;
    else
        return time(NULL) - 60 * 60 * cutoff_hours;
}

static void twitter_buddy_change_state(PurpleBuddy * buddy, gboolean online, const gchar * message)
{
    if (online == PURPLE_BUDDY_IS_ONLINE(buddy))
        return;
    purple_prpl_got_user_status(purple_buddy_get_account(buddy), buddy->name, online ? TWITTER_STATUS_ONLINE : TWITTER_STATUS_OFFLINE, "message", message, NULL);
}

static void twitter_buddy_touch_state_with_cutoff(PurpleBuddy * buddy, time_t cutoff)
{
    PurpleAccount  *account = purple_buddy_get_account(buddy);
    TwitterUserTweet *user_tweet = twitter_buddy_get_buddy_data(buddy);
    TwitterTweet   *tweet = user_tweet ? user_tweet->status : NULL;
    gchar          *tweet_message = tweet ? tweet->text : NULL;

#ifdef _HAZE_
    //Haze has chats as buddies. Keep them always online
    if (buddy->name && (buddy->name[0] == '#' || twitter_usernames_match(account, buddy->name, "Timeline: Home"))) {
        twitter_buddy_change_state(buddy, TRUE, tweet_message);
        return;
    }
#endif
    //Yes, I know this could be 'shorter'. But this is (somewhat) clearer
    if (!cutoff)                                 //Always set buddies to online
    {
        if (twitter_option_get_following(account) && !user_tweet) {
            //This user was added to the buddy list, but we aren't following them
            //set the user offline
            twitter_buddy_change_state(buddy, FALSE, tweet_message);
        } else {
            //Either get_following is true and the user_tweet has been seen, or
            //get_following is false, so we just set them to online
            twitter_buddy_change_state(buddy, TRUE, tweet_message);
        }
    } else {
        if (!tweet || tweet->created_at < cutoff) {
            //No tweet or the tweet was created before the cutoff, set offline
            twitter_buddy_change_state(buddy, FALSE, tweet_message);
        } else {
            //Tweet after the cutoff, set to online
            twitter_buddy_change_state(buddy, TRUE, tweet_message);
        }
    }
}

void twitter_buddy_touch_state(PurpleBuddy * buddy)
{
    PurpleAccount  *account = purple_buddy_get_account(buddy);
    time_t          cutoff = twitter_account_get_online_cutoff(account);
    twitter_buddy_touch_state_with_cutoff(buddy, cutoff);
}

void twitter_buddy_touch_state_all(PurpleAccount * account)
{
    GSList         *buddies;
    GSList         *b_node;
    time_t          cutoff = twitter_account_get_online_cutoff(account);

    if (!cutoff)
        return;

    buddies = purple_find_buddies(account, NULL);

    for (b_node = buddies; b_node; b_node = g_slist_next(b_node)) {
        twitter_buddy_touch_state_with_cutoff((PurpleBuddy *) b_node->data, cutoff);
    }

    g_slist_free(buddies);
}

void twitter_buddy_set_status_data(PurpleAccount * account, const char *src_user, TwitterTweet * s)
{
    PurpleBuddy    *b;
    TwitterUserTweet *buddy_data;
    gboolean        status_text_same = FALSE;
    time_t          cutoff = twitter_account_get_online_cutoff(account);

    if (!s)
        return;

    if (!s->text) {
        twitter_status_data_free(s);
        return;
    }

    b = purple_find_buddy(account, src_user);
    if (!b) {
        twitter_status_data_free(s);
        return;
    }

    buddy_data = twitter_buddy_get_buddy_data(b);

    if (buddy_data->status && s->created_at < buddy_data->status->created_at) {
        twitter_status_data_free(s);
        return;
    }

    if (buddy_data->status != NULL && s != buddy_data->status) {
        status_text_same = (strcmp(buddy_data->status->text, s->text) == 0);
        twitter_status_data_free(buddy_data->status);
    }

    buddy_data->status = s;

    if (!status_text_same) {
        purple_prpl_got_user_status(b->account, b->name, cutoff && s && s->created_at < cutoff ? TWITTER_STATUS_OFFLINE : TWITTER_STATUS_ONLINE, "message", s ? s->text : NULL, NULL);
    }
}

PurpleBuddy    *twitter_buddy_new(PurpleAccount * account, const char *screenname, const char *alias)
{
    PurpleGroup    *g;
    PurpleBuddy    *b = purple_find_buddy(account, screenname);
    TwitterUserTweet *twitter_buddy;
    const char     *group_name;
    if (b != NULL) {
        if (b->proto_data == NULL) {
            b->proto_data = twitter_user_tweet_new(screenname, NULL, NULL, NULL);
        }
        return b;
    }

    group_name = twitter_option_buddy_group(account);
    g = purple_find_group(group_name);
    if (g == NULL)
        g = purple_group_new(group_name);
    b = purple_buddy_new(account, screenname, alias);
    purple_blist_add_buddy(b, NULL, g, NULL);
    twitter_buddy = twitter_user_tweet_new(screenname, NULL, NULL, NULL);
    b->proto_data = twitter_buddy;
    return b;
}

void twitter_buddy_set_user_data(PurpleAccount * account, TwitterUserData * u, gboolean add_missing_buddy)
{
    PurpleBuddy    *b;
    TwitterUserTweet *buddy_data;
    char          **userparts = g_strsplit(purple_account_get_username(account), "@", 2);
    const char     *sn = userparts[0];
    if (!u || !account) {
        g_strfreev(userparts);
        return;
    }

    if (!strcmp(u->screen_name, sn)) {
        g_strfreev(userparts);
        twitter_user_data_free(u);
        return;
    }
    g_strfreev(userparts);

    b = purple_find_buddy(account, u->screen_name);
    if (!b && add_missing_buddy) {
        /* set alias as screenname (name) */
        const gchar    *alias_type = twitter_option_alias_format(account);
        gchar          *alias;
        if (!strcmp(alias_type, TWITTER_PREF_ALIAS_FORMAT_FULLNAME)) {
            alias = g_strdup_printf("%s", u->name);
        } else if (!strcmp(alias_type, TWITTER_PREF_ALIAS_FORMAT_NICK)) {
            alias = g_strdup_printf("%s", u->screen_name);
        } else {
            alias = g_strdup_printf("%s | %s", u->name, u->screen_name);
        }
        b = twitter_buddy_new(account, u->screen_name, alias);
        g_free(alias);
    }

    if (!b) {
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

typedef struct {
    PurpleAccount  *account;
    gchar          *buddy_name;
    gchar          *url;
} BuddyIconContext;

static void twitter_buddy_update_icon_cb(PurpleUtilFetchUrlData * url_data, gpointer user_data, const gchar * url_text, gsize len, const gchar * error_message)
{
    BuddyIconContext *b = user_data;
    PurpleBuddyIcon *buddy_icon;
    purple_buddy_icons_set_for_user(b->account, b->buddy_name, g_memdup(url_text, len), len, b->url);

    if ((buddy_icon = purple_buddy_icons_find(b->account, b->buddy_name))) {
        purple_signal_emit(purple_buddy_icons_get_handle(), "prpltwtr-update-buddyicon", b->account, b->buddy_name, buddy_icon);
        purple_buddy_icon_unref(buddy_icon);
    }

    g_free(b->buddy_name);
    g_free(b->url);
    g_free(b);
}

void twitter_buddy_update_icon_from_username(PurpleAccount * account, const gchar * username, const gchar * url)
{
    const gchar    *previous_url;
    PurpleBuddyIcon *icon;
    if (url == NULL) {
        purple_buddy_icons_set_for_user(account, username, NULL, 0, NULL);
        return;
    }

    if ((icon = purple_buddy_icons_find(account, username))) {
        previous_url = purple_buddy_icon_get_checksum(icon);
        purple_buddy_icon_unref(icon);
    } else {
        previous_url = NULL;
    }

    if (previous_url == NULL || !g_str_equal(previous_url, url)) {
        BuddyIconContext *b = g_new0(BuddyIconContext, 1);
        b->account = account;
        b->buddy_name = g_strdup(username);
        b->url = g_strdup(url);

        purple_buddy_icons_set_for_user(account, username, NULL, 0, url);

        purple_signal_emit(purple_buddy_icons_get_handle(), "prpltwtr-update-buddyicon", account, username, NULL);

        purple_util_fetch_url_request_len_with_account(account, url, TRUE, NULL, FALSE, NULL, FALSE, -1, twitter_buddy_update_icon_cb, b);

    }
}

void twitter_buddy_update_icon(PurpleBuddy * buddy)
{
    TwitterUserTweet *twitter_buddy_data = buddy->proto_data;
    if (twitter_buddy_data == NULL || twitter_buddy_data->user == NULL) {
        return;
    } else {
        const char     *url = twitter_buddy_data->user->profile_image_url;
        twitter_buddy_update_icon_from_username(buddy->account, buddy->name, url);
    }
}
