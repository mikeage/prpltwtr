#include "prpltwtr_request.h"
#include "prpltwtr_conn.h"
void            prpltwtr_auth_invalidate_token(PurpleAccount * account);
void            prpltwtr_auth_oauth_login(PurpleAccount * account, TwitterConnectionData * twitter);

void            prpltwtr_auth_pre_send_auth_basic(TwitterRequestor * r, gboolean * post, const char **url, TwitterRequestParams ** params, gchar *** header_fields, gpointer * requestor_data);
void            prpltwtr_auth_post_send_auth_basic(TwitterRequestor * r, gboolean * post, const char **url, TwitterRequestParams ** params, gchar *** header_fields, gpointer * requestor_data);
void            prpltwtr_auth_pre_send_oauth(TwitterRequestor * r, gboolean * post, const char **url, TwitterRequestParams ** params, gchar *** header_fields, gpointer * requestor_data);
void            prpltwtr_auth_post_send_oauth(TwitterRequestor * r, gboolean * post, const char **url, TwitterRequestParams ** params, gchar *** header_fields, gpointer * requestor_data);
const gchar    *prpltwtr_auth_get_oauth_key(PurpleAccount * account);
const gchar    *prpltwtr_auth_get_oauth_secret(PurpleAccount * account);
