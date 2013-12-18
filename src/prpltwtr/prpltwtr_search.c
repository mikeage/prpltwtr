/**
 * Copyright (C) 2009 Tan Miaoqing
 * Contact: Tan Miaoqing <rabbitrun84@gmail.com>
 */

#include "defaults.h"

#include <glib.h>
#include <glib/gprintf.h>

#include <string.h>
#include <debug.h>
#include <request.h>

#include "prpltwtr_prefs.h"
#include "prpltwtr_util.h"
#include "prpltwtr_search.h"
#include "prpltwtr_request.h"

typedef struct {
    TwitterSearchSuccessFunc success_func;
    TwitterSearchErrorFunc error_func;
    gpointer        user_data;
} TwitterSearchContext;

static void twitter_send_search_success_cb(TwitterRequestor * r, gpointer response_node, gpointer user_data)
{
    TwitterSearchContext *ctx = user_data;
    TwitterSearchResults *results = twitter_search_results_node_parse(r, response_node);

    ctx->success_func(r->account, results->tweets, results->refresh_url, results->max_id, ctx->user_data);

    results->tweets = NULL;

    twitter_search_results_free(results);
    g_slice_free(TwitterSearchContext, ctx);
}

void twitter_search(TwitterRequestor * r, TwitterRequestParams * params, TwitterSearchSuccessFunc success_cb, TwitterSearchErrorFunc error_cb, gpointer data)
{
    TwitterSearchContext *ctx;
    ctx = g_slice_new0(TwitterSearchContext);
    ctx->user_data = data;
    ctx->success_func = success_cb;
    ctx->error_func = error_cb;
    twitter_send_format_request(r, FALSE, r->urls->get_search_results, params, twitter_send_search_success_cb, NULL,    //TODO error
                                ctx);

}
