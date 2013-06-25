#ifndef _PT_API_H_
#define _PT_API_H_

#include "pt_requestor.h"

void pt_api_verify_credentials(PtRequestor * r, PtSendJsonRequestSuccessFunc success_cb, PtSendRequestErrorFunc error_cb, gpointer user_data);

#endif
