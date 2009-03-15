
void twitter_api_get_friends(PurpleAccount *account,
		TwitterSendRequestFunc success_func,
		TwitterSendRequestFunc error_func);

void twitter_api_get_replies(PurpleAccount *account,
		unsigned int since_id,
		TwitterSendRequestFunc success_func,
		TwitterSendRequestFunc error_func);

void twitter_api_get_rate_limit_status(PurpleAccount *account,
		TwitterSendRequestFunc success_func,
		TwitterSendRequestFunc error_func);

void twitter_api_set_status(PurpleAccount *acct, 
		const char *msg,
		TwitterSendRequestFunc success_func,
		TwitterSendRequestFunc error_func,
		gpointer data);
