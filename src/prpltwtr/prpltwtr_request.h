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

#ifndef _TWITTER_REQUEST_H_
#define _TWITTER_REQUEST_H_

#include <glib.h>
#include "prpltwtr_plugin.h"
#include "prpltwtr_format.h"

typedef struct {
    gchar          *name;
    gchar          *value;
} TwitterRequestParam;

typedef struct _TwitterRequestor TwitterRequestor;

TwitterRequestParam *twitter_request_param_new(const gchar * name, const gchar * value);
TwitterRequestParam *twitter_request_param_new_int(const gchar * name, int value);
TwitterRequestParam *twitter_request_param_new_ll(const gchar * name, long long value);

typedef GArray  TwitterRequestParams;

TwitterRequestParams *twitter_request_params_new(void);
TwitterRequestParams *twitter_request_params_add(GArray * params, TwitterRequestParam * p);
void            twitter_request_params_free(TwitterRequestParams * params);
void            twitter_request_param_free(TwitterRequestParam * p);

typedef enum {
    TWITTER_REQUEST_ERROR_NONE,
    TWITTER_REQUEST_ERROR_SERVER,
    TWITTER_REQUEST_ERROR_TWITTER_GENERAL,
    TWITTER_REQUEST_ERROR_INVALID_FORMAT,
    TWITTER_REQUEST_ERROR_NO_OAUTH,
    TWITTER_REQUEST_ERROR_CANCELED,

    TWITTER_REQUEST_ERROR_UNAUTHORIZED
} TwitterRequestErrorType;

typedef enum _TwitterRequestType {
	TWITTER_REQUEST_TYPE_UNTRACKED_REQUEST,
    TWITTER_REQUEST_TYPE_GET_RATE_LIMIT_STATUS,
    TWITTER_REQUEST_TYPE_GET_FRIENDS,
    TWITTER_REQUEST_TYPE_GET_HOME_TIMELINE,
    TWITTER_REQUEST_TYPE_GET_MENTIONS,
    TWITTER_REQUEST_TYPE_GET_DMS,
    TWITTER_REQUEST_TYPE_UPDATE_STATUS,
    TWITTER_REQUEST_TYPE_NEW_DM,
    TWITTER_REQUEST_TYPE_GET_SAVED_SEARCHES,
    TWITTER_REQUEST_TYPE_GET_SUBSCRIBED_LISTS,
    TWITTER_REQUEST_TYPE_GET_PERSONAL_LISTS,
    TWITTER_REQUEST_TYPE_GET_SEARCH_RESULTS,
    TWITTER_REQUEST_TYPE_VERIFY_CREDENTIALS,
    TWITTER_REQUEST_TYPE_REPORT_SPAMMER,
    TWITTER_REQUEST_TYPE_ADD_FAVORITE,
    TWITTER_REQUEST_TYPE_DELETE_FAVORITE,
    TWITTER_REQUEST_TYPE_GET_USER_INFO
} TwitterRequestType;

typedef struct {
    TwitterRequestErrorType type;
    /*const xmlnode *response_node; */
    const gchar    *message;
} TwitterRequestErrorData;

typedef void    (*TwitterSendRequestSuccessFunc) (TwitterRequestor * r, const gchar * response, gpointer user_data);

typedef void    (*TwitterSendXmlRequestSuccessFunc) (TwitterRequestor * r, xmlnode * node, gpointer user_data);

/// The signature for a successful callback that doesn't matter if it is an `xmlnode` or
/// a `JsonNode` or any other format-specific element.
typedef void    (*TwitterSendFormatRequestSuccessFunc) (TwitterRequestor * r, gpointer node, gpointer user_data);

typedef void    (*TwitterSendRequestErrorFunc) (TwitterRequestor * r, const TwitterRequestErrorData * error_data, gpointer user_data);

struct _TwitterRequestor {
    PurpleAccount  *account;
    GList          *pending_requests;

    void            (*pre_send) (TwitterRequestor * r, gboolean * post, const char **url, TwitterRequestParams ** params, gchar *** header_fields, gpointer * requestor_data);
                    gpointer(*do_send) (TwitterRequestor * r, TwitterRequestType request_type, gboolean post, const char *url, TwitterRequestParams * params, char **header_fields, TwitterSendRequestSuccessFunc success_callback, TwitterSendRequestErrorFunc error_callback, gpointer data);
    void            (*post_send) (TwitterRequestor * r, gboolean * post, const char **url, TwitterRequestParams ** params, gchar *** header_fields, gpointer * requestor_data);
                    gboolean(*pre_failed) (TwitterRequestor * r, const TwitterRequestErrorData ** error_data);
    void            (*post_failed) (TwitterRequestor * r, const TwitterRequestErrorData ** error_data);
    TwitterUrls    *urls;
    TwitterFormat  *format;
};

void            twitter_requestor_free(TwitterRequestor * requestor);

int             xmlnode_child_count(xmlnode * parent);

typedef struct _TwitterMultiPageRequestData TwitterMultiPageRequestData;
typedef struct _TwitterFormatMultiPageRequestData TwitterFormatMultiPageRequestData;

typedef         gboolean(*TwitterSendRequestMultiPageSuccessFunc) (TwitterRequestor * r, xmlnode * node, gboolean last_page, TwitterMultiPageRequestData * request, gpointer user_data);
typedef         gboolean(*TwitterSendFormatRequestMultiPageSuccessFunc) (TwitterRequestor * r, gpointer node, gboolean last_page, TwitterFormatMultiPageRequestData * request, gpointer user_data);
typedef         gboolean(*TwitterSendRequestMultiPageErrorFunc) (TwitterRequestor * r, const TwitterRequestErrorData * error_data, gpointer user_data);

struct _TwitterMultiPageRequestData {
    gpointer        user_data;
    char           *url;
    //char *query_string;
    TwitterRequestParams *params;
    TwitterSendRequestMultiPageSuccessFunc success_callback;
    TwitterSendRequestMultiPageErrorFunc error_callback;
    int             page;
    int             expected_count;
};

struct _TwitterFormatMultiPageRequestData {
    gpointer        user_data;
    char           *url;
    //char *query_string;
    TwitterRequestParams *params;
    TwitterSendFormatRequestMultiPageSuccessFunc success_callback;
    TwitterSendRequestMultiPageErrorFunc error_callback;
    int             page;
    int             expected_count;
};

typedef void    (*TwitterSendRequestMultiPageAllSuccessFunc) (TwitterRequestor * r, GList * nodes, gpointer user_data);
typedef void    (*TwitterSendFormatRequestMultiPageAllSuccessFunc) (TwitterRequestor * r, GList * nodes, gpointer user_data);
typedef         gboolean(*TwitterSendRequestMultiPageAllErrorFunc) (TwitterRequestor * r, const TwitterRequestErrorData * error_data, gpointer user_data);

void            prpltwtr_requestor_post_failed(TwitterRequestor * r, const TwitterRequestErrorData ** error_data);
gpointer        twitter_requestor_send(TwitterRequestor * r, TwitterRequestType request_type, gboolean post, const char *url, TwitterRequestParams * params, char **header_fields, TwitterSendRequestSuccessFunc success_callback, TwitterSendRequestErrorFunc error_callback, gpointer data);

void            twitter_send_request(TwitterRequestor * r, TwitterRequestType request_type, gboolean post, const char *url, TwitterRequestParams * params, TwitterSendRequestSuccessFunc success_callback, TwitterSendRequestErrorFunc error_callback, gpointer data);

void            twitter_send_xml_request(TwitterRequestor * r, gboolean post, const char *url, TwitterRequestParams * params, TwitterSendXmlRequestSuccessFunc success_callback, TwitterSendRequestErrorFunc error_callback, gpointer data);

void            twitter_send_format_request(TwitterRequestor * r, TwitterRequestType, gboolean post, const char *url, TwitterRequestParams * params, TwitterSendFormatRequestSuccessFunc success_callback, TwitterSendRequestErrorFunc error_callback, gpointer data);

//don't include count in the query_string
void            twitter_send_xml_request_multipage_all(TwitterRequestor * r, const char *url, TwitterRequestParams * params, TwitterSendRequestMultiPageAllSuccessFunc success_callback, TwitterSendRequestMultiPageAllErrorFunc error_callback, int expected_count, gint max_count, gpointer data);

void            twitter_send_format_request_multipage_all(TwitterRequestor * r, const char *url, TwitterRequestParams * params, TwitterSendRequestMultiPageAllSuccessFunc success_callback, TwitterSendRequestMultiPageAllErrorFunc error_callback, int expected_count, gint max_count, gpointer data);

/* statuses/friends API deprecated page based retrieval,
 * and use cursor based method instead */
void            twitter_send_xml_request_with_cursor(TwitterRequestor * r, const char *url, TwitterRequestParams * params, gchar * cursor, TwitterSendRequestMultiPageAllSuccessFunc success_callback, TwitterSendRequestMultiPageAllErrorFunc error_callback, gpointer data);

void            twitter_send_format_request_with_cursor(TwitterRequestor * r, const char *url, TwitterRequestParams * params, long long cursor, TwitterSendRequestMultiPageAllSuccessFunc success_callback, TwitterSendRequestMultiPageAllErrorFunc error_callback, gpointer data);

TwitterRequestParams *twitter_request_params_add_oauth_params(PurpleAccount * account, gboolean post, const gchar * url, const TwitterRequestParams * params, const gchar * token, const gchar * signing_key);

const gchar    *twitter_response_text_data(const gchar * response_text, gsize len);
gint            twitter_response_text_status_code(const gchar * response_text);

#endif
