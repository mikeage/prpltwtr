#ifndef _PT_REQUESTOR_H_
#define _PT_REQUESTOR_H_

#include <glib.h>

#include "prpl.h"

typedef struct _PtRequestor PtRequestor;

typedef struct
{
	gchar          *name;
	gchar          *value;
} PtRequestorParam;
typedef GArray  PtRequestorParams;

PtRequestorParam *pt_requestor_param_new_str (const gchar * name, const gchar * value);
PtRequestorParam *pt_requestor_param_new_int (const gchar * name, int value);
PtRequestorParam *pt_requestor_param_new_ll (const gchar * name, long long value);

PtRequestorParams *pt_requestor_params_new (void);
PtRequestorParams *pt_requestor_params_add (PtRequestorParams * params, PtRequestorParam * param);
void            pt_requestor_param_free (PtRequestorParam * p);
void            pt_requestor_params_free (PtRequestorParams * params);

PtRequestorParam *pt_requestor_param_clone (const PtRequestorParam * p);
PtRequestorParams *pt_requestor_params_clone (const PtRequestorParams * params);

typedef enum
{
	PT_REQUESTOR_ERROR_NONE,
	PT_REQUESTOR_ERROR_SERVER,
	PT_REQUESTOR_ERROR_TWITTER_GENERAL,
	PT_REQUESTOR_ERROR_INVALID_XML,
	PT_REQUESTOR_ERROR_NO_OAUTH,
	PT_REQUESTOR_ERROR_CANCELED,

	PT_REQUESTOR_ERROR_UNAUTHORIZED
} PtRequestorError;

typedef struct
{
	PtRequestorError error;
	const gchar    *message;
} PtRequestorErrorData;

typedef void    (*PtSendRequestSuccessFunc) (PtRequestor * r, const gchar * response, gpointer user_data);

//typedef void    (*PtSendXmlRequestSuccessFunc) (PtRequestor * r, xmlnode * node, gpointer user_data);
typedef void    (*PtSendRequestErrorFunc) (PtRequestor * r, const PtRequestorErrorData * error_data, gpointer user_data);

struct _PtRequestor
{
	PurpleAccount  *account;
	GList          *pending_requests;

	void            (*pre_send) (PtRequestor * r, gboolean * post, const char **url, PtRequestorParams ** params, gchar *** header_fields, gpointer * requestor_data);
	                gpointer (*do_send) (PtRequestor * r, gboolean post, const char *url, PtRequestorParams * params, char **header_fields, PtSendRequestSuccessFunc success_callback, PtSendRequestErrorFunc error_callback, gpointer data);
	void            (*post_send) (PtRequestor * r, gboolean * post, const char **url, PtRequestorParams ** params, gchar *** header_fields, gpointer * requestor_data);
	                gboolean (*pre_failed) (PtRequestor * r, const PtRequestorErrorData ** error_data);
	void            (*post_failed) (PtRequestor * r, const PtRequestorErrorData ** error_data);
	int             rate_limit_total;
	int             rate_limit_remaining;
};

void            twitter_requestor_free (PtRequestor * requestor);

//typedef struct _TwitterMultiPageRequestData TwitterMultiPageRequestData;

//typedef         gboolean(*PtSendRequestMultiPageSuccessFunc) (PtRequestor * r, xmlnode * node, gboolean last_page, TwitterMultiPageRequestData * request, gpointer user_data);
//typedef         gboolean(*PtSendRequestMultiPageErrorFunc) (PtRequestor * r, const PtRequestorErrorData * error_data, gpointer user_data);

//struct _TwitterMultiPageRequestData {
    //gpointer        user_data;
    //char           *url;
    ////char *query_string;
    //PtRequestorParams *params;
    //PtSendRequestMultiPageSuccessFunc success_callback;
    //PtSendRequestMultiPageErrorFunc error_callback;
    //int             page;
    //int             expected_count;
//};

//typedef void    (*PtSendRequestMultiPageAllSuccessFunc) (PtRequestor * r, GList * nodes, gpointer user_data);
//typedef         gboolean(*PtSendRequestMultiPageAllErrorFunc) (PtRequestor * r, const PtRequestorErrorData * error_data, gpointer user_data);

gpointer        pt_requestor_send (PtRequestor * r, gboolean post, const char *url, PtRequestorParams * params, char **header_fields, PtSendRequestSuccessFunc success_callback, PtSendRequestErrorFunc error_callback, gpointer data);

void            pt_send_request (PtRequestor * r, gboolean post, const char *url, PtRequestorParams * params, PtSendRequestSuccessFunc success_callback, PtSendRequestErrorFunc error_callback, gpointer data);

//void            twitter_send_xml_request(PtRequestor * r, gboolean post, const char *url, PtRequestorParams * params, PtSendXmlRequestSuccessFunc success_callback, PtSendRequestErrorFunc error_callback, gpointer data);

////don't include count in the query_string
//void            twitter_send_xml_request_multipage_all(PtRequestor * r, const char *url, PtRequestorParams * params, PtSendRequestMultiPageAllSuccessFunc success_callback, PtSendRequestMultiPageAllErrorFunc error_callback, int expected_count, gint max_count, gpointer data);

/* statuses/friends API deprecated page based retrieval,
 * and use cursor based method instead */
//void            twitter_send_xml_request_with_cursor(PtRequestor * r, const char *url, PtRequestorParams * params, long long cursor, PtSendRequestMultiPageAllSuccessFunc success_callback, PtSendRequestMultiPageAllErrorFunc error_callback, gpointer data);

PtRequestorParams *pt_requestor_params_add_oauth_params (PurpleAccount * account, gboolean post, const gchar * url, const PtRequestorParams * params, const gchar * token, const gchar * signing_key);

//const gchar    *twitter_response_text_data(const gchar * response_text, gsize len);
//gint            twitter_response_text_status_code(const gchar * response_text);

PtRequestor    *pt_requestor_get_requestor (PurpleAccount * account);
gint            pt_requestor_get_status_code (const gchar * response_text);
const gchar    *pt_requestor_get_text (const gchar * response_text, gsize len);

typedef struct
{
	PtRequestor    *requestor;
	PtSendRequestSuccessFunc success_func;
	PtSendRequestErrorFunc error_func;

	gpointer        request_id;
	gpointer        user_data;
} PtSendRequestData;

#endif
