/**
 * Copyright (C) 2009 Tan Miaoqing
 * Contact: Tan Miaoqing <rabbitrun84@gmail.com>
 */

#include "config.h"

#include <glib.h>
#include <glib/gprintf.h>

#include <string.h>
#include <debug.h>
#include <request.h>

#include "twitter_prefs.h"
#include "twitter_util.h"
#include "twitter_search.h"
#include "twitter_request.h"

typedef struct {
	PurpleAccount *account;
	TwitterSearchSuccessFunc success_func;
	TwitterSearchErrorFunc error_func;
	gpointer user_data;
} TwitterSearchContext;

static void _free_search_results (GArray *search_results)
{
	guint i, len;

	if (!search_results)
		return ;

	len = search_results->len;

	for (i = 0; i < len; i++) {
		TwitterUserTweet *search_data;

		search_data = g_array_index (search_results,
				TwitterUserTweet *, i);
		twitter_user_tweet_free(search_data);
	}
	g_array_free (search_results, TRUE);
}

static TwitterUserTweet *twitter_search_entry_node_parse(xmlnode *entry_node)
{
	if (entry_node != NULL && entry_node->name && !strcmp(entry_node->name, "entry"))
	{
		TwitterUserTweet *entry;
		TwitterTweet *tweet = g_new0(TwitterTweet, 1);
		gchar *id_str = xmlnode_get_child_data(entry_node, "id"); //tag:search.twitter.com,2005:12345678
		gchar *created_at_str = xmlnode_get_child_data(entry_node, "published"); //2009-12-24T19:29:24Z
		gchar *screen_name_str = xmlnode_get_child_data(xmlnode_get_child(entry_node, "author"), "name"); //username (USER NAME)
		gchar *ptr;


		ptr = g_strrstr(id_str, ":");
		if (ptr != NULL)
		{
			tweet->id = strtoll(ptr + 1, NULL, 10);
		}
		ptr = strstr(screen_name_str, " ");
		if (ptr)
			ptr[0] = 0;
		entry = twitter_user_tweet_new(screen_name_str, NULL, NULL);
		g_free(screen_name_str);

		tweet->text = xmlnode_get_child_data(entry_node, "title");
		tweet->created_at = purple_str_to_time(created_at_str, TRUE, NULL, NULL, NULL);
		entry->status = tweet;

		g_free(id_str);
		g_free(created_at_str);

		return entry;
	}
	return NULL;
}

static gint _twitter_search_results_sort(gconstpointer _a, gconstpointer _b)
{
	long long a = (*((TwitterUserTweet **) _a))->status->id;
	long long b = (*((TwitterUserTweet **) _b))->status->id;
	if (a < b)
		return 1;
	else if (a > b)
		return -1;
	else
		return 0;
}

static void twitter_send_search_success_cb(PurpleAccount *acct, xmlnode *response_node, gpointer user_data)
{
	TwitterSearchContext *ctx = user_data;
	GArray *search_results = NULL;
	const gchar *refresh_url = NULL;
	long long max_id = 0; /* id of last search result */
	xmlnode *entry_node;
	xmlnode *link_node;
	const gchar *ptr;

	search_results = g_array_new (FALSE, FALSE, sizeof (TwitterUserTweet *));

	for (link_node = xmlnode_get_child(response_node, "link"); link_node; link_node = xmlnode_get_next_twin(link_node))
	{
		const char *rel = xmlnode_get_attrib(link_node, "rel");
		if (rel != NULL && !strcmp(rel, "refresh"))
		{
			const char *refresh_url_full = xmlnode_get_attrib(link_node, "href");
			ptr = strstr(refresh_url_full, "?");
			if (ptr != NULL)
			{
				refresh_url = ptr;
				break;
			}
		}
	}
	for (entry_node = xmlnode_get_child(response_node, "entry"); entry_node; entry_node = xmlnode_get_next_twin(entry_node))
	{
		TwitterUserTweet *entry = twitter_search_entry_node_parse(entry_node);
		if (entry != NULL)
		{
			g_array_append_val(search_results, entry);
			if (max_id < entry->status->id)
				max_id = entry->status->id;
		}
	}

	g_array_sort(search_results, _twitter_search_results_sort);

	purple_debug_info(TWITTER_PROTOCOL_ID, "refresh_url: %s, max_id: %lld\n",
			refresh_url, max_id);

	ctx->success_func (ctx->account, search_results,
			refresh_url, max_id, ctx->user_data);

	_free_search_results (search_results);
	g_slice_free (TwitterSearchContext, ctx);
}

void twitter_search (PurpleAccount *account, const char *query,
		TwitterSearchSuccessFunc success_cb, TwitterSearchErrorFunc error_cb,
		gpointer data)
{
	const gchar *search_url = twitter_option_url_get_search_results(account);
	TwitterSearchContext *ctx;
	if (search_url && search_url[0] != '\0')
	{
		ctx = g_slice_new0 (TwitterSearchContext);
		ctx->account = account;
		ctx->user_data = data;
		ctx->success_func = success_cb;
		ctx->error_func = error_cb;
		twitter_send_request(account, FALSE,
				search_url, query,
				twitter_send_search_success_cb, NULL, //TODO error
				ctx);

	}
}
