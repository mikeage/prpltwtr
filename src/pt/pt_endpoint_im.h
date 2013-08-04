#ifndef _PT_ENDPOINT_IM_H_
#define _PT_ENDPOINT_IM_H_

#include "defines.h"

#include "pt_api.h"
#include "pt_requestor.h"

typedef enum
{
	PT_IM_TYPE_AT_MSG = 0,
	PT_IM_TYPE_DM = 1,
	PT_IM_TYPE_UNKNOWN = 2,
} PtImType;

typedef void    (*PtApiImAllFunc) (PtRequestor * r, long long since_id, PtSendRequestMultiPageAllSuccessFunc success_func, PtSendRequestMultiPageAllErrorFunc error_func, gint max_count, gpointer data);

typedef struct
{
	PtImType        type;
	const gchar    *since_id_setting_id;
	const gchar    *conv_id;
	int             (*send_im) (PurpleAccount * account, const char *buddy_name, const char *message, PurpleMessageFlags flags);
	int             (*timespan_func) (PurpleAccount * account);
	PtApiImAllFunc  get_im_func;
	PtSendRequestMultiPageAllSuccessFunc success_cb;
	PtSendRequestMultiPageAllErrorFunc error_cb;
	void            (*get_last_since_id) (PurpleAccount * account, void (*success_cb) (PurpleAccount * account, long long id, gpointer user_data), void (*error_cb) (PurpleAccount * account, const PtRequestorErrorData * error_data, gpointer user_data), gpointer user_data);
	void            (*convo_closed) (PurpleConversation * conv);
} PtEndpointImSettings;

typedef struct
{
	PurpleAccount  *account;
	long long       since_id;
	gboolean        retrieve_history;
	gint            initial_max_retrieve;
	PtEndpointImSettings *settings;

	//these should be 'private'
	guint           timer;
	gboolean        ran_once;
} PtEndpointIm;

PtEndpointIm   *pt_endpoint_im_new (PurpleAccount * account, PtEndpointImSettings * settings, gboolean retrieve_history, gint initial_max_retrieve);
void            pt_endpoint_im_free (PtEndpointIm * ctx);

PtEndpointIm   *pt_endpoint_im_find (PurpleAccount * account, PtImType type);
PtEndpointIm   *pt_conv_name_to_endpoint_im (PurpleAccount * account, const char *name);
const char     *pt_conv_name_to_buddy_name (PurpleAccount * account, const char *name);

void            pt_endpoint_im_settings_save_since_id (PurpleAccount * account, PtEndpointImSettings * settings, long long since_id);
long long       pt_endpoint_im_settings_load_since_id (PurpleAccount * account, PtEndpointImSettings * settings);
void            pt_endpoint_im_set_since_id (PtEndpointIm * ctx, long long since_id);
long long       pt_endpoint_im_get_since_id (PtEndpointIm * ctx);

void            pt_endpoint_im_start (PtEndpointIm * ctx);
char           *pt_endpoint_im_buddy_name_to_conv_name (PtEndpointIm * im, const char *name);
void            pt_endpoint_im_update_conv (PtEndpointIm * ctx, char *buddy_name, void /* PtTweet */ * s);
PtImType        pt_conv_name_to_type (PurpleAccount * account, const char *name);
void            pt_endpoint_im_convo_closed (PtEndpointIm * im, const gchar * conv_name);

#endif
