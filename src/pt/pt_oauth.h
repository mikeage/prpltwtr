#ifndef _PT_OAUTH_H_
#define _PT_OAUTH_H_

#include "prpl.h"

#include "pt_connection.h"
//#include "prpltwtr_request.h"
//#include "prpltwtr_conn.h"
//void            prpltwtr_auth_invalidate_token(PurpleAccount * account);
void            pt_oauth_login(PurpleAccount * account, PtConnectionData * conn_data);

void            pt_oauth_pre_send(PtRequestor * requestor, gboolean * post, const char **url, PtRequestorParams ** params, gchar *** header_fields, gpointer * requestor_data);
void            pt_oauth_post_send(PtRequestor * requestor, gboolean * post, const char **url, PtRequestorParams ** params, gchar *** header_fields, gpointer * requestor_data);
#endif 
