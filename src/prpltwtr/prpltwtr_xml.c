#include <json-glib/json-glib.h>

#include "prpltwtr_xml.h"
const gchar    *twitter_account_get_last_home_timeline_id(PurpleAccount * account);
TwitterUserTweet *twitter_search_entry_node_parse(TwitterRequestor * r, gpointer entry_node);

static time_t twitter_get_timezone_offset()
{
    static long     tzoff = PURPLE_NO_TZ_OFF;
    if (tzoff == PURPLE_NO_TZ_OFF) {
        struct tm       t;
        time_t          tval = 0;
        const char     *tzoff_str;
        long            tzoff_l;

        tzoff = 0;

        time(&tval);
        localtime_r(&tval, &t);
        tzoff_str = purple_get_tzoff_str(&t, FALSE);
        if (tzoff_str && tzoff_str[0] != '\0') {
            tzoff_l = strtol(tzoff_str, NULL, 10);
            tzoff = (60 * 60 * (int) (tzoff_l / 100)) + (60 * (tzoff_l % 100));
        }
    }
    return tzoff;
}

static time_t twitter_status_parse_timestamp(const char *timestamp)
{
    //Sat Mar 07 18:12:10 +0000 2009
    char            month_str[4],
                    tz_str[6];
    char            day_name[4];
    char           *tz_ptr = tz_str;
    static const char *months[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", NULL
    };
    time_t          tval = 0;
    struct tm       t;
    memset(&t, 0, sizeof (t));

    time(&tval);
    localtime_r(&tval, &t);

    if (sscanf(timestamp, "%03s %03s %02d %02d:%02d:%02d %05s %04d", day_name, month_str, &t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec, tz_str, &t.tm_year) == 8) {
        //gboolean        offset_positive = TRUE;
        int             tzhrs;
        int             tzmins;

        for (t.tm_mon = 0; months[t.tm_mon] != NULL && strcmp(months[t.tm_mon], month_str) != 0; t.tm_mon++) ;
        if (months[t.tm_mon] != NULL) {
            if (*tz_str == '-') {
                //offset_positive = FALSE;
                tz_ptr++;
            } else if (*tz_str == '+') {
                tz_ptr++;
            }

            t.tm_year -= 1900;

            if (sscanf(tz_ptr, "%02d%02d", &tzhrs, &tzmins) == 2) {
                time_t          tzoff = tzhrs * 60 * 60 + tzmins * 60;
                time_t          returned_time;
                tzoff += twitter_get_timezone_offset();

                returned_time = mktime(&t);
                if (returned_time != -1 && returned_time != 0)
                    return returned_time + tzoff;
            }
        }
    }

    purple_debug_error(GENERIC_PROTOCOL_ID, "Can't parse timestamp %s\n", timestamp);
    return tval;
}

static gint _twitter_search_results_sort(TwitterUserTweet * _a, TwitterUserTweet * _b)
{
    gchar          *a = _a->status->id;
    gchar          *b = _b->status->id;
    if (a < b)
        return -1;
    else if (a > b)
        return 1;
    else
        return 0;
}

static const gchar *twitter_search_entry_get_icon_url(TwitterRequestor * r, gpointer entry_node)
{
    gpointer        link_node = r->format->iter_start(entry_node, "link");
    for (; !r->format->iter_done(entry_node) && strcmp(r->format->get_attr(link_node, "rel"), "image"); link_node = r->format->iter_next(link_node)) {
    }
    if (link_node)
        return r->format->get_attr(link_node, "href");
    return NULL;
}

TwitterUserTweet *twitter_search_entry_node_parse(TwitterRequestor * r, gpointer entry_node)
{
    if (entry_node != NULL && r->format->is_name(entry_node, "entry")) {
        TwitterUserTweet *entry;
        TwitterTweet   *tweet = g_new0(TwitterTweet, 1);
        gchar          *id_str = r->format->get_str(entry_node, "id");
        gchar          *created_at_str = r->format->get_str(entry_node, "published");
        gchar          *screen_name_str = r->format->get_str(xmlnode_get_child(entry_node, "author"), "name");
        const gchar    *icon_url;
        gchar          *ptr;

        ptr = g_strrstr(id_str, ":");
        if (ptr != NULL) {
            tweet->id = ptr + 1;
        }
        ptr = strstr(screen_name_str, " ");
        if (ptr)
            ptr[0] = 0;
        icon_url = twitter_search_entry_get_icon_url(r, entry_node);
        entry = twitter_user_tweet_new(screen_name_str, icon_url, NULL, NULL);

        tweet->text = r->format->get_str(entry_node, "title");
        tweet->created_at = purple_str_to_time(created_at_str, TRUE, NULL, NULL, NULL);
        entry->status = tweet;

        g_free(id_str);
        g_free(created_at_str);
        g_free(screen_name_str);

        return entry;
    }
    return NULL;
}

static TwitterSearchResults *twitter_search_results_new(GList * tweets, gchar * refresh_url, gchar * max_id)
{
    TwitterSearchResults *results = g_new(TwitterSearchResults, 1);
    results->refresh_url = refresh_url;
    results->tweets = tweets;
    results->max_id = max_id;

    return results;
}

void twitter_search_results_free(TwitterSearchResults * results)
{
    if (!results)
        return;
    if (results->refresh_url)
        g_free(results->refresh_url);
    if (results->tweets) {
        GList          *l;
        for (l = results->tweets; l; l = l->next) {
            if (l->data)
                twitter_user_tweet_free((TwitterUserTweet *) l->data);
        }
        g_list_free(results->tweets);
    }
    g_free(results);
}

TwitterSearchResults *twitter_search_results_node_parse(TwitterRequestor * r, gpointer response_node)
{
    GList          *search_results = NULL;
    const gchar    *refresh_url = NULL;
    gchar          *max_id = 0; // id of last search result
    gpointer       *link_node = NULL;
    gpointer        iter;
    gpointer        entry_node;
    const gchar    *ptr;

    for (iter = r->format->iter_start(response_node, "link"); !r->format->iter_done(iter); iter = r->format->iter_next(iter)) {
        const char     *rel = NULL;
        link_node = r->format->get_iter_node(iter);
        rel = r->format->get_attr(link_node, "rel");
        if (rel != NULL && !strcmp(rel, "refresh")) {
            const char     *refresh_url_full = r->format->get_attr(link_node, "href");
            ptr = strstr(refresh_url_full, "?");
            if (ptr != NULL) {
                refresh_url = ptr;
                break;
            }
        }
    }

    /* After snowflake, the IDs aren't sequential; always take the first entry */
    for (entry_node = r->format->iter_start(response_node, "entry"); !r->format->iter_done(entry_node); entry_node = r->format->iter_next(entry_node)) {
        TwitterUserTweet *entry = twitter_search_entry_node_parse(r, entry_node);
        if (entry != NULL) {
            search_results = g_list_append(search_results, entry);
            if (!max_id) {
                max_id = entry->status->id;
            }
        }
    }

    //TODO: test and remove
    search_results = g_list_sort(search_results, (GCompareFunc) _twitter_search_results_sort);

    purple_debug_info(GENERIC_PROTOCOL_ID, "refresh_url: %s, max_id: %s\n", refresh_url, max_id);

    return twitter_search_results_new(search_results, g_strdup(refresh_url), max_id);
}

TwitterUserData *twitter_user_node_parse(TwitterRequestor * r, gpointer user_node)
{
    TwitterUserData *user;
    TwitterFormat  *format = r->format;

    if (user_node == NULL)
        return NULL;

    user = g_new0(TwitterUserData, 1);
    user->screen_name = format->get_str(user_node, "screen_name");

    if (!user->screen_name) {
        purple_debug_info("prpltwtr/user_node_parse", "Cannot find screen name, skipping\n");
        g_free(user);
        return NULL;
    }

    user->name = format->get_str(user_node, "name");
    user->profile_image_url = format->get_str(user_node, "profile_image_url");

    user->id = format->get_str(user_node, "id_str");    // Need a generic way of handling this.

    purple_debug_info("prpltwtr/user_node_parse", "Loading user: %s (%s, %s)\n", user->screen_name, user->name, user->id);

    user->statuses_count = format->get_str(user_node, "statuses_count");
    user->friends_count = format->get_str(user_node, "friends_count");
    user->followers_count = format->get_str(user_node, "followers_count");
    user->description = format->get_str(user_node, "description");

#if 0
    {
        char           *xmlnode_str;
        int             len;
        xmlnode_str = xmlnode_to_str(user_node, &len);
        purple_debug_info(TWITTER_PROTOCOL_ID, "Parsing user node: |%s|\n", xmlnode_str);
        g_free(xmlnode_str);
    }
#endif

    return user;
}

TwitterTweet   *twitter_status_node_parse(TwitterRequestor * r, gpointer status_node)
{
    TwitterTweet   *status;
    TwitterFormat  *format = r->format;
    gchar          *data;
    gpointer       *retweeted_status_node = NULL;

    if (status_node == NULL)
        return NULL;

    status = g_new0(TwitterTweet, 1);
    status->text = format->get_str(status_node, "text");

    purple_debug_info("prprltwtr/status_node_parse", "Status: %s\n", status->text);

    if ((data = format->get_str(status_node, "created_at"))) {
        time_t          created_at = twitter_status_parse_timestamp(data);
        status->created_at = created_at ? created_at : time(NULL);
        g_free(data);
    }

    if ((data = format->get_str(status_node, "id_str"))) {
        status->id = data;
    }

    if ((data = format->get_str(status_node, "in_reply_to_status_id_str"))) {
        status->in_reply_to_status_id = data;
    }

    if ((data = format->get_str(status_node, "favorited"))) {
        status->favorited = !strcmp(data, "true") ? TRUE : FALSE;
        g_free(data);
    } else {
        status->favorited = FALSE;
    }
    status->in_reply_to_screen_name = format->get_str(status_node, "in_reply_to_screen_name");

    if ((retweeted_status_node = format->get_node(status_node, "retweeted_status"))) {
        gchar          *rt_text;
        gpointer       *rt_user_node;
        gchar          *rt_screen_name;
        rt_text = format->get_str(retweeted_status_node, "text");
        if ((rt_user_node = format->get_node(retweeted_status_node, "user"))) {
            rt_screen_name = format->get_str(rt_user_node, "screen_name");
            // We don't need the original text, since it's cut off
            g_free(status->text);
            status->text = g_strconcat("RT @", rt_screen_name, ": ", rt_text, NULL);
            g_free(rt_screen_name);
        }
        g_free(rt_text);
    }

    return status;
}

TwitterUserTweet *twitter_update_status_node_parse(TwitterRequestor * r, gpointer update_status_node)
{
    TwitterTweet   *tweet = twitter_status_node_parse(r, update_status_node);
    TwitterUserData *user;
    gpointer        child_node = NULL;
    if (!tweet)
        return NULL;
    child_node = r->format->get_node(update_status_node, "user");
    user = twitter_user_node_parse(r, child_node);
    if (!user) {
        twitter_status_data_free(tweet);
        return NULL;
    }
    return twitter_user_tweet_new(user->screen_name, user->profile_image_url, user, tweet);
}

TwitterUserTweet *twitter_verify_credentials_parse(TwitterRequestor * r, gpointer node)
{
    TwitterUserData *user = twitter_user_node_parse(r, node);
    TwitterTweet   *tweet;
    TwitterUserTweet *data;
    gpointer        child_node = NULL;
    if (!user)
        return NULL;

    child_node = r->format->get_node(node, "status");
    tweet = twitter_status_node_parse(r, child_node);
    data = twitter_user_tweet_new(user->screen_name, user->profile_image_url, user, tweet);

    return data;
}

TwitterUserTweet *twitter_user_tweet_new(const char *screen_name, const gchar * icon_url, TwitterUserData * user, TwitterTweet * tweet)
{
    TwitterUserTweet *data = g_new0(TwitterUserTweet, 1);

    data->user = user;
    data->status = tweet;
    data->screen_name = g_strdup(screen_name);
    if (icon_url)
        data->icon_url = g_strdup(icon_url);

    return data;
}

TwitterUserData *twitter_user_tweet_take_user_data(TwitterUserTweet * ut)
{
    TwitterUserData *data = ut->user;
    ut->user = NULL;
    return data;
}

TwitterTweet   *twitter_user_tweet_take_tweet(TwitterUserTweet * ut)
{
    TwitterTweet   *data = ut->status;
    ut->status = NULL;
    return data;
}

void twitter_user_tweet_free(TwitterUserTweet * ut)
{
    if (!ut)
        return;
    if (ut->user)
        twitter_user_data_free(ut->user);
    if (ut->status)
        twitter_status_data_free(ut->status);
    if (ut->screen_name)
        g_free(ut->screen_name);
    if (ut->icon_url)
        g_free(ut->icon_url);
    g_free(ut);
    ut = NULL;
}

GList          *twitter_dms_node_parse(TwitterRequestor * r, gpointer dms_node)
{
    GList          *dms = NULL;
    gpointer       *dm_node;
    gpointer        iter;

    purple_debug_info(GENERIC_PROTOCOL_ID, "%s: END\n", G_STRFUNC);

    if (JSON_NODE_TYPE(dms_node) == JSON_NODE_ARRAY) {
        for (iter = r->format->iter_start(dms_node, NULL); !r->format->iter_done(iter); iter = r->format->iter_next(iter)) {
            dm_node = r->format->get_iter_node(iter);

            if (dm_node != NULL) {
                if (r->format->is_name(dm_node, "status")) {
                    TwitterUserData *user = twitter_user_node_parse(r, r->format->get_node(dm_node, "sender"));
                    TwitterTweet   *tweet = twitter_status_node_parse(r, dm_node);
                    TwitterUserTweet *data = twitter_user_tweet_new(user->screen_name, user->profile_image_url, user, tweet);

                    dms = g_list_prepend(dms, data);
                }
            }
        }
    } else if (JSON_NODE_TYPE(dms_node) == JSON_NODE_OBJECT) {
        // TODO Utter violation of the format.
        TwitterUserData *user = twitter_user_node_parse(r, r->format->get_node(dms_node, "sender"));
        TwitterTweet   *tweet = twitter_status_node_parse(r, dms_node);
        TwitterUserTweet *data = twitter_user_tweet_new(user->screen_name, user->profile_image_url, user, tweet);

        purple_debug_info(GENERIC_PROTOCOL_ID, "%s: object: %s\n", G_STRFUNC, tweet->text);
        dms = g_list_prepend(dms, data);
    }

    return dms;
}

GList          *twitter_dms_nodes_parse(TwitterRequestor * r, GList * nodes)
{
    GList          *l_users_data = NULL;
    GList          *l;
    for (l = nodes; l; l = l->next) {
        gpointer       *node = l->data;
        l_users_data = g_list_concat(l_users_data, twitter_dms_node_parse(r, node));
    }
    return l_users_data;
}

GList          *twitter_users_node_parse(TwitterRequestor * r, gpointer users_node)
{
    GList          *users = NULL;
    /* TODO
       gpointer        user_node;
       for (user_node = users_node->child; user_node; user_node = user_node->next) {
       if (user_node->name && !strcmp(user_node->name, "user")) {
       TwitterUserData *user = twitter_user_node_parse(user_node);
       TwitterTweet   *tweet = twitter_dm_node_parse(xmlnode_get_child(user_node, "status"));
       TwitterUserTweet *data = twitter_user_tweet_new(user->screen_name, user->profile_image_url, user, tweet);

       users = g_list_append(users, data);
       }
       }
     */

    return users;
}

GList          *twitter_users_ids_nodes_parse(TwitterRequestor * r, GList * nodes)
{
    GList          *l_users = NULL;
    /* TODO
       xmlnode        *ids;
       xmlnode        *id;
       if (nodes && nodes->data) {
       ids = xmlnode_get_child((nodes->data), "ids");
       if (ids) {
       for (id = xmlnode_get_child(ids, "id"); id; id = xmlnode_get_next_twin(id)) {
       l_users = g_list_prepend(l_users, xmlnode_get_data_unescaped(id));
       }
       }
       }
       if (!l_users) {
       purple_debug_warning(TWITTER_PROTOCOL_ID, "Empty nodes list!\n");
       }
     */
    return l_users;
}

GList          *twitter_users_nodes_parse(TwitterRequestor * r, GList * nodes)
{
    GList          *l_users_data = NULL;
    GList          *l;

    for (l = nodes; l; l = l->next) {
        gpointer        node = l->data;
        l_users_data = g_list_append(l_users_data, twitter_users_node_parse(r, node));
    }

    return l_users_data;
}

GList          *twitter_statuses_node_parse(TwitterRequestor * r, gpointer statuses_node)
{
    GList          *statuses = NULL;
    gpointer        status_node;
    gpointer        iter;

    purple_debug_info(GENERIC_PROTOCOL_ID, "%s: BEGIN array %d object %d value %d\n", G_STRFUNC, JSON_NODE_TYPE(statuses_node) == JSON_NODE_ARRAY, JSON_NODE_TYPE(statuses_node) == JSON_NODE_OBJECT, JSON_NODE_TYPE(statuses_node) == JSON_NODE_VALUE);

    if (JSON_NODE_TYPE(statuses_node) == JSON_NODE_ARRAY) {
        for (iter = r->format->iter_start(statuses_node, NULL); !r->format->iter_done(iter); iter = r->format->iter_next(iter)) {
            status_node = r->format->get_iter_node(iter);

            if (status_node != NULL) {
                if (r->format->is_name(status_node, "status")) {
                    TwitterUserData *user = twitter_user_node_parse(r, r->format->get_node(status_node, "user"));
                    TwitterTweet   *tweet = twitter_status_node_parse(r, status_node);
                    TwitterUserTweet *data = twitter_user_tweet_new(user->screen_name, user->profile_image_url, user, tweet);

                    statuses = g_list_prepend(statuses, data);
                }
            }
        }
    } else if (JSON_NODE_TYPE(statuses_node) == JSON_NODE_OBJECT) {
        // TODO Utter violation of the format.
        TwitterUserData *user = twitter_user_node_parse(r, r->format->get_node(statuses_node, "user"));
        TwitterTweet   *tweet = twitter_status_node_parse(r, statuses_node);
        TwitterUserTweet *data = twitter_user_tweet_new(user->screen_name, user->profile_image_url, user, tweet);

        purple_debug_info(GENERIC_PROTOCOL_ID, "%s: object: %s\n", G_STRFUNC, tweet->text);
        statuses = g_list_prepend(statuses, data);
    }

    purple_debug_info(GENERIC_PROTOCOL_ID, "%s: END\n", G_STRFUNC);

    return statuses;
}

GList          *twitter_statuses_nodes_parse(TwitterRequestor * r, GList * nodes)
{
    GList          *l_users_data = NULL;
    GList          *l;
    for (l = nodes; l; l = l->next) {
        gpointer       *node = l->data;
        l_users_data = g_list_concat(l_users_data, twitter_statuses_node_parse(r, node));
    }
    return l_users_data;
}

void twitter_user_data_free(TwitterUserData * user_data)
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
    if (user_data->statuses_count)
        g_free(user_data->statuses_count);
    if (user_data->friends_count)
        g_free(user_data->friends_count);
    if (user_data->followers_count)
        g_free(user_data->followers_count);

    g_free(user_data);
    user_data = NULL;
}

void twitter_status_data_free(TwitterTweet * status)
{
    if (status == NULL)
        return;

    if (status->text != NULL)
        g_free(status->text);
    status->text = NULL;

    if (status->in_reply_to_screen_name)
        g_free(status->in_reply_to_screen_name);
    status->in_reply_to_screen_name = NULL;

    g_free(status);
}
