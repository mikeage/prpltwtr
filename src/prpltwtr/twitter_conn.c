#include "twitter_conn.h"

void twitter_connection_foreach_endpoint_im(TwitterConnectionData *twitter,
		void (*cb)(TwitterConnectionData *twitter, TwitterEndpointIm *im, gpointer data),
		gpointer data)
{
	int i;
	for (i = 0; i < TWITTER_IM_TYPE_UNKNOWN; i++)
		if (twitter->endpoint_ims[i])
			cb(twitter, twitter->endpoint_ims[i], data);
}

TwitterEndpointIm *twitter_connection_get_endpoint_im(TwitterConnectionData *twitter, TwitterImType type)
{
	if (type < TWITTER_IM_TYPE_UNKNOWN)
		return twitter->endpoint_ims[type];
	return NULL;
}

void twitter_connection_set_endpoint_im(TwitterConnectionData *twitter, TwitterImType type, TwitterEndpointIm *endpoint)
{
	twitter->endpoint_ims[type] = endpoint;
}

TwitterRequestor* purple_account_get_requestor(PurpleAccount *account)
{
	PurpleConnection *gc = purple_account_get_connection(account);
	TwitterConnectionData *twitter = gc->proto_data;
	return twitter->requestor;
}
