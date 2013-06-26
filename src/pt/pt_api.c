#include "config.h"

#include "pt_api.h"

void pt_api_verify_credentials(PtRequestor * r, PtSendJsonRequestSuccessFunc success_cb, PtSendRequestErrorFunc error_cb, gpointer user_data)
{
    pt_send_json_request(r, FALSE, "api.twitter.com/1.1/account/verify_credentials.json", NULL, success_cb, error_cb, user_data);
}

