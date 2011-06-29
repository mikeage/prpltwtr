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

#include <glib/gstdio.h>

#include "prpltwtr.h"

typedef struct {
    PurpleAccount  *account;
    gchar          *username;
} TwitterAccountUserNameChange;

static GHashTable *oauth_result_to_hashtable(const gchar * txt);
static void     account_mismatch_screenname_change_cancel_cb(TwitterAccountUserNameChange * change, gint action_id);
static void     account_mismatch_screenname_change_ok_cb(TwitterAccountUserNameChange * change, gint action_id);
static void     account_username_change_verify(PurpleAccount * account, const gchar * username);
static void     verify_credentials_success_cb(TwitterRequestor * r, xmlnode * node, gpointer user_data);
static void     verify_credentials_error_cb(TwitterRequestor * r, const TwitterRequestErrorData * error_data, gpointer user_data);
static void     oauth_request_token_success_cb(TwitterRequestor * r, const gchar * response, gpointer user_data);
static void     oauth_request_token_error_cb(TwitterRequestor * r, const TwitterRequestErrorData * error_data, gpointer user_data);
static const gchar *account_get_oauth_access_token(PurpleAccount * account);
static void     account_set_oauth_access_token(PurpleAccount * account, const gchar * oauth_token);
static const gchar *account_get_oauth_access_token_secret(PurpleAccount * account);
static void     account_set_oauth_access_token_secret(PurpleAccount * account, const gchar * oauth_token);
static void     oauth_request_pin_ok(PurpleAccount * account, const gchar * pin);
static void     oauth_request_pin_cancel(PurpleAccount * account, const gchar * pin);
static void     oauth_access_token_success_cb(TwitterRequestor * r, const gchar * response, gpointer user_data);
static void     oauth_access_token_error_cb(TwitterRequestor * r, const TwitterRequestErrorData * error_data, gpointer user_data);
static const gchar *twitter_option_url_oauth_authorize(PurpleAccount * account);
static const gchar *twitter_option_url_oauth_access_token(PurpleAccount * account);
static const gchar *twitter_option_url_oauth_request_token(PurpleAccount * account);
static const gchar *twitter_oauth_create_url(PurpleAccount * account, const gchar * endpoint);

void prpltwtr_auth_invalidate_token(PurpleAccount * account)
{
    account_set_oauth_access_token(account, NULL);
    account_set_oauth_access_token_secret(account, NULL);
}

void prpltwtr_auth_pre_send_auth_basic(TwitterRequestor * r, gboolean * post, const char **url, TwitterRequestParams ** params, gchar *** header_fields, gpointer * requestor_data)
{
    const char     *pass = purple_connection_get_password(purple_account_get_connection(r->account));
    char          **userparts = g_strsplit(purple_account_get_username(r->account), "@", 2);
    const char     *sn = userparts[0];
    char           *auth_text = g_strdup_printf("%s:%s", sn, pass);
    char           *auth_text_b64 = purple_base64_encode((guchar *) auth_text, strlen(auth_text));
    *header_fields = g_new(gchar *, 2);

    (*header_fields)[0] = g_strdup_printf("Authorization: Basic %s", auth_text_b64);
    (*header_fields)[1] = NULL;

    g_strfreev(userparts);
    g_free(auth_text);
    g_free(auth_text_b64);
}

void prpltwtr_auth_post_send_auth_basic(TwitterRequestor * r, gboolean * post, const char **url, TwitterRequestParams ** params, gchar *** header_fields, gpointer * requestor_data)
{
    g_strfreev(*header_fields);
}

const gchar    *prpltwtr_auth_get_oauth_key(PurpleAccount * account)
{
    if (!strcmp(purple_account_get_protocol_id(account), TWITTER_PROTOCOL_ID)) {
        return TWITTER_OAUTH_KEY;
    } else {
        const gchar    *key = purple_account_get_string(account, TWITTER_PREF_CONSUMER_KEY, "");
        if (!strcmp(key, "")) {
            purple_debug_error(purple_account_get_protocol_id(account), "No Consumer key specified!\n");
        }
        return key;
    }
}

const gchar    *prpltwtr_auth_get_oauth_secret(PurpleAccount * account)
{
    if (!strcmp(purple_account_get_protocol_id(account), TWITTER_PROTOCOL_ID)) {
        return TWITTER_OAUTH_SECRET;
    } else {
        const gchar    *secret = purple_account_get_string(account, TWITTER_PREF_CONSUMER_SECRET, "");
        if (!strcmp(secret, "")) {
            purple_debug_error(purple_account_get_protocol_id(account), "No Consumer secret specified!\n");
        }
        return secret;
    }
}

void prpltwtr_auth_pre_send_oauth(TwitterRequestor * r, gboolean * post, const char **url, TwitterRequestParams ** params, gchar *** header_fields, gpointer * requestor_data)
{
    PurpleAccount  *account = r->account;
    PurpleConnection *gc = purple_account_get_connection(account);
    TwitterConnectionData *twitter = gc->proto_data;
    gchar          *signing_key = g_strdup_printf("%s&%s",
                                                  prpltwtr_auth_get_oauth_secret(account),
                                                  twitter->oauth_token_secret ? twitter->oauth_token_secret : "");
    TwitterRequestParams *oauth_params = twitter_request_params_add_oauth_params(account, *post, *url,
                                                                                 *params, twitter->oauth_token, signing_key);

    if (oauth_params == NULL) {
        TwitterRequestErrorData *error = g_new0(TwitterRequestErrorData, 1);
        gchar          *error_msg = g_strdup(_("Could not sign request"));
        error->type = TWITTER_REQUEST_ERROR_NO_OAUTH;
        error->message = error_msg;
        g_free(error_msg);
        g_free(error);
        g_free(signing_key);
        //TODO: error if couldn't sign
        return;
    }

    g_free(signing_key);

    *requestor_data = *params;
    *params = oauth_params;
}

void prpltwtr_auth_oauth_login(PurpleAccount * account, TwitterConnectionData * twitter)
{
    const gchar    *oauth_token;
    const gchar    *oauth_token_secret;
    oauth_token = account_get_oauth_access_token(account);
    oauth_token_secret = account_get_oauth_access_token_secret(account);
    if (oauth_token && oauth_token_secret) {
        twitter->oauth_token = g_strdup(oauth_token);
        twitter->oauth_token_secret = g_strdup(oauth_token_secret);
        twitter_api_verify_credentials(purple_account_get_requestor(account), verify_credentials_success_cb, verify_credentials_error_cb, NULL);
    } else {
        twitter_send_request(purple_account_get_requestor(account), FALSE, twitter_option_url_oauth_request_token(account), NULL, oauth_request_token_success_cb, oauth_request_token_error_cb, NULL);
    }
}

void prpltwtr_auth_post_send_oauth(TwitterRequestor * r, gboolean * post, const char **url, TwitterRequestParams ** params, gchar *** header_fields, gpointer * requestor_data)
{
    twitter_request_params_free(*params);
    *params = (TwitterRequestParams *) * requestor_data;
}

static void oauth_access_token_success_cb(TwitterRequestor * r, const gchar * response, gpointer user_data)
{
    PurpleAccount  *account = r->account;
    PurpleConnection *gc = purple_account_get_connection(account);
    TwitterConnectionData *twitter = gc->proto_data;

    GHashTable     *results = oauth_result_to_hashtable(response);
    const gchar    *oauth_token = g_hash_table_lookup(results, "oauth_token");
    const gchar    *oauth_token_secret = g_hash_table_lookup(results, "oauth_token_secret");
    const gchar    *response_screen_name = g_hash_table_lookup(results, "screen_name");
    if (oauth_token && oauth_token_secret) {
        if (twitter->oauth_token)
            g_free(twitter->oauth_token);
        if (twitter->oauth_token_secret)
            g_free(twitter->oauth_token_secret);

        twitter->oauth_token = g_strdup(oauth_token);
        twitter->oauth_token_secret = g_strdup(oauth_token_secret);

        account_set_oauth_access_token(account, oauth_token);
        account_set_oauth_access_token_secret(account, oauth_token_secret);

        //FIXME: set this to be case insensitive
        {
            char          **userparts = g_strsplit(purple_account_get_username(r->account), "@", 2);
            const char     *username = userparts[0];
            if (response_screen_name && !twitter_usernames_match(account, response_screen_name, username)) {
                account_username_change_verify(account, response_screen_name);
            } else {
                prpltwtr_verify_connection(account);
            }
            g_strfreev(userparts);
        }
    } else {
        purple_debug_error(purple_account_get_protocol_id(account), "Unknown error receiving access token: %s\n", response);
        prpltwtr_disconnect(account, _("Unknown response getting access token"));
    }
}

static void oauth_access_token_error_cb(TwitterRequestor * r, const TwitterRequestErrorData * error_data, gpointer user_data)
{
    gchar          *error = g_strdup_printf(_("Error verifying PIN: %s"), error_data->message ? error_data->message : _("unknown error"));
    prpltwtr_disconnect(r->account, error);
    g_free(error);
}

static GHashTable *oauth_result_to_hashtable(const gchar * txt)
{
    gchar         **pieces = g_strsplit(txt, "&", 0);
    GHashTable     *results = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    gchar         **p;

    for (p = pieces; *p; p++) {
        gchar          *equalpos = strchr(*p, '=');
        if (equalpos) {
            equalpos[0] = '\0';
            g_hash_table_replace(results, g_strdup(*p), g_strdup(equalpos + 1));
        }
    }
    g_strfreev(pieces);
    return results;
}

static void account_mismatch_screenname_change_cancel_cb(TwitterAccountUserNameChange * change, gint action_id)
{
    PurpleAccount  *account = change->account;
    prpltwtr_auth_invalidate_token(account);
    g_free(change->username);
    g_free(change);
    prpltwtr_disconnect(account, _("Username mismatch"));
}

static void account_mismatch_screenname_change_ok_cb(TwitterAccountUserNameChange * change, gint action_id)
{
    PurpleAccount  *account = change->account;
    purple_account_set_username(account, change->username);
    g_free(change->username);
    g_free(change);
    prpltwtr_verify_connection(account);
}

static void account_username_change_verify(PurpleAccount * account, const gchar * username)
{
    PurpleConnection *gc = purple_account_get_connection(account);
    gchar          *secondary = g_strdup_printf(_("Do you wish to change the name on this account to %s?"), username);
    TwitterAccountUserNameChange *change_data = (TwitterAccountUserNameChange *) g_new0(TwitterAccountUserNameChange *, 1);

    change_data->account = account;
    change_data->username = g_strdup(username);

    purple_request_action(gc, _("Mismatched Screen Names"), _("Authorized screen name does not match with account screen name"), secondary, 0, account, NULL, NULL, change_data, 2, _("Cancel"), account_mismatch_screenname_change_cancel_cb, _("Yes"), account_mismatch_screenname_change_ok_cb, NULL);

    g_free(secondary);
}

static void verify_credentials_success_cb(TwitterRequestor * r, xmlnode * node, gpointer user_data)
{
    PurpleAccount  *account = r->account;
    TwitterUserTweet *user_tweet = twitter_verify_credentials_parse(node);
    char          **userparts = g_strsplit(purple_account_get_username(r->account), "@", 2);
    const char     *username = userparts[0];

    if (!user_tweet || !user_tweet->screen_name) {
        prpltwtr_disconnect(account, _("Could not verify credentials"));
    } else if (!twitter_usernames_match(account, user_tweet->screen_name, username)) {
        account_username_change_verify(account, user_tweet->screen_name);
    } else {
        prpltwtr_verify_connection(account);
    }
    g_strfreev(userparts);
    twitter_user_tweet_free(user_tweet);
}

static void verify_credentials_error_cb(TwitterRequestor * r, const TwitterRequestErrorData * error_data, gpointer user_data)
{
    gchar          *error = g_strdup_printf(_("Error verifying credentials: %s"), error_data->message ? error_data->message : _("unknown error"));
    switch (error_data->type) {
    case TWITTER_REQUEST_ERROR_SERVER:
        prpltwtr_recoverable_disconnect(r->account, error);
        break;
    case TWITTER_REQUEST_ERROR_NONE:
    case TWITTER_REQUEST_ERROR_TWITTER_GENERAL:
    case TWITTER_REQUEST_ERROR_INVALID_XML:
    case TWITTER_REQUEST_ERROR_NO_OAUTH:
    case TWITTER_REQUEST_ERROR_CANCELED:
    case TWITTER_REQUEST_ERROR_UNAUTHORIZED:
    default:
        prpltwtr_disconnect(r->account, error);
        break;
    }
    g_free(error);
}

static void oauth_request_token_success_cb(TwitterRequestor * r, const gchar * response, gpointer user_data)
{
    PurpleAccount  *account = r->account;
    PurpleConnection *gc = purple_account_get_connection(account);
    TwitterConnectionData *twitter = gc->proto_data;

    GHashTable     *results = oauth_result_to_hashtable(response);
    const gchar    *oauth_token = g_hash_table_lookup(results, "oauth_token");
    const gchar    *oauth_token_secret = g_hash_table_lookup(results, "oauth_token_secret");
    if (oauth_token && oauth_token_secret) {
/* http://twitter.com/oauth/authorize */
        gchar          *msg = g_strdup_printf("http://%s?oauth_token=%s",
                                              twitter_option_url_oauth_authorize(account),
                                              purple_url_encode(oauth_token));
        gchar          *prompt = g_strdup_printf("%s %s", _("Please enter PIN for"), purple_account_get_username(account));

        twitter->oauth_token = g_strdup(oauth_token);
        twitter->oauth_token_secret = g_strdup(oauth_token_secret);
        purple_notify_uri(twitter, msg);

        purple_request_input(twitter, _("OAuth Authentication"),    //title
                             prompt,             //primary
                             msg,                //secondary
                             NULL,               //default
                             FALSE,              //multiline,
                             FALSE,              //password
                             NULL,               //hint
                             _("Submit"),        //ok text
                             G_CALLBACK(oauth_request_pin_ok), _("Cancel"), G_CALLBACK(oauth_request_pin_cancel), account, NULL, NULL, account);
        g_free(msg);
        g_free(prompt);
    } else {
        purple_debug_error(purple_account_get_protocol_id(account), "Unknown error receiving request token: %s\n", response);
        prpltwtr_disconnect(account, _("Invalid response receiving request token"));
    }
    g_hash_table_destroy(results);
}

static void oauth_request_token_error_cb(TwitterRequestor * r, const TwitterRequestErrorData * error_data, gpointer user_data)
{
    gchar          *error = g_strdup_printf(_("Error receiving request token: %s"), error_data->message ? error_data->message : _("unknown error"));
    prpltwtr_disconnect(r->account, error);
    g_free(error);
}

static const gchar *account_get_oauth_access_token(PurpleAccount * account)
{
    return purple_account_get_string(account, "oauth_token", NULL);
}

static void account_set_oauth_access_token(PurpleAccount * account, const gchar * oauth_token)
{
    purple_account_set_string(account, "oauth_token", oauth_token);
}

static const gchar *account_get_oauth_access_token_secret(PurpleAccount * account)
{
    return purple_account_get_string(account, "oauth_token_secret", NULL);
}

static void account_set_oauth_access_token_secret(PurpleAccount * account, const gchar * oauth_token)
{
    purple_account_set_string(account, "oauth_token_secret", oauth_token);
}

static void oauth_request_pin_ok(PurpleAccount * account, const gchar * pin)
{
    TwitterRequestParams *params = twitter_request_params_new();
    twitter_request_params_add(params, twitter_request_param_new("oauth_verifier", pin));
    twitter_send_request(purple_account_get_requestor(account), FALSE, twitter_option_url_oauth_access_token(account), params, oauth_access_token_success_cb, oauth_access_token_error_cb, NULL);
    twitter_request_params_free(params);
}

static void oauth_request_pin_cancel(PurpleAccount * account, const gchar * pin)
{
    prpltwtr_disconnect(account, _("Canceled PIN entry"));
}

static const gchar *twitter_option_url_oauth_authorize(PurpleAccount * account)
{
    return twitter_oauth_create_url(account, "/authorize");
}

static const gchar *twitter_option_url_oauth_request_token(PurpleAccount * account)
{
    return twitter_oauth_create_url(account, "/request_token");
}

static const gchar *twitter_option_url_oauth_access_token(PurpleAccount * account)
{
    return twitter_oauth_create_url(account, "/access_token");
}

static const gchar *twitter_oauth_create_url(PurpleAccount * account, const gchar * endpoint)
{
    static char     url[1024];
    char            host[256];

    g_return_val_if_fail(endpoint != NULL && endpoint[0] != '\0', NULL);

    if (!strcmp(purple_account_get_protocol_id(account), TWITTER_PROTOCOL_ID)) {
        snprintf(host, 255, "twitter.com/oauth");
    } else {
        snprintf(host, 255, "%s/oauth", purple_account_get_string(account, TWITTER_PREF_API_BASE, STATUSNET_PREF_API_BASE_DEFAULT));
    }

    snprintf(url, 1023, "%s%s%s", host, host[strlen(host) - 1] == '/' || endpoint[0] == '/' ? "" : "/", host[strlen(host) - 1] == '/' && endpoint[0] == '/' ? endpoint + 1 : endpoint);
    return url;
}
