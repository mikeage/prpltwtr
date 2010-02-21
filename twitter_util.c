/**
 * Copyright (C) 2009 Tan Miaoqing
 * Contact: Tan Miaoqing <rabbitrun84@gmail.com>
 */

#include "twitter_util.h"

gboolean twitter_usernames_match(PurpleAccount *account, const gchar *u1, const gchar *u2)
{
	gboolean match;
	gchar *u1n = g_strdup(purple_normalize(account, u1));
	const gchar *u2n = purple_normalize(account, u2);
	match = !strcmp(u1n, u2n);

	g_free(u1n);
	return match;
}


long long purple_account_get_long_long(PurpleAccount *account, const gchar *key, long long default_value)
{
	const char* tmp_str;

	tmp_str = purple_account_get_string(account, key, NULL);
	if(tmp_str)
		return strtoll(tmp_str, NULL, 10);
	else
		return default_value;
}

void purple_account_set_long_long(PurpleAccount *account, const gchar *key, long long value)
{
	gchar* tmp_str;

	tmp_str = g_strdup_printf("%lld", value);
	purple_account_set_string(account, key, tmp_str);
	g_free(tmp_str);
}


//TODO: move those
#if _HAVE_PIDGIN_
static const char *_find_first_delimiter(const char *text, const char *delimiters, int *delim_id)
{
	const char *delimiter;
	if (text == NULL || text[0] == '\0')
		return NULL;
	do
	{
		for (delimiter = delimiters; *delimiter != '\0'; delimiter++)
		{
			if (*text == *delimiter)
			{
				if (delim_id != NULL)
					*delim_id = delimiter - delimiters;
				return text;
			}
		}
	} while (*++text != '\0');
	return NULL;
}
#endif

//TODO: move those
static char *twitter_linkify(PurpleAccount *account, const char *message)
{
#if _HAVE_PIDGIN_
	GString *ret;
	static char symbols[] = "#@";
	static char *symbol_actions[] = {TWITTER_URI_ACTION_SEARCH, TWITTER_URI_ACTION_USER};
	static char delims[] = " :"; //I don't know if this is how I want to do this...
	const char *ptr = message;
	const char *end = message + strlen(message);
	const char *delim = NULL;
	g_return_val_if_fail(message != NULL, NULL);

	ret = g_string_new("");

	while (ptr != NULL && ptr < end)
	{
		const char *first_token = NULL;
		char *current_action = NULL;
		char *link_text = NULL;
		int symbol_index = 0;
		first_token = _find_first_delimiter(ptr, symbols, &symbol_index);
		if (first_token == NULL)
		{
			g_string_append(ret, ptr);
			break;
		}
		current_action = symbol_actions[symbol_index];
		g_string_append_len(ret, ptr, first_token - ptr);
		ptr = first_token;
		delim = _find_first_delimiter(ptr, delims, NULL);
		if (delim == NULL)
			delim = end;
		link_text = g_strndup(ptr, delim - ptr);
		//Added the 'a' before the account name because of a highlighting issue... ugly hack
		g_string_append_printf(ret, "<a href=\"" TWITTER_URI ":///%s?account=a%s&text=%s\">%s</a>",
				current_action,
				purple_account_get_username(account),
				purple_url_encode(link_text),
				purple_markup_escape_text(link_text, -1));
		ptr = delim;
	}

	return g_string_free(ret, FALSE);
#else
	return g_strdup(message);
#endif
}

//TODO: move those
char *twitter_format_tweet(PurpleAccount *account,
		const char *src_user,
		const char *message,
		long long tweet_id,
		PurpleConversationType conv_type,
		const gchar *conv_name,
		gboolean is_tweet)
{
	char *linkified_message = twitter_linkify(account, message);
	GString *tweet;

	g_return_val_if_fail(linkified_message != NULL, NULL);
	g_return_val_if_fail(src_user != NULL, NULL);

	tweet = g_string_new(linkified_message);

#if _HAVE_PIDGIN_
	if (is_tweet && tweet_id && conv_type != PURPLE_CONV_TYPE_UNKNOWN && conv_name)
	{
		const gchar *account_name = purple_account_get_username(account);
		//TODO: make this an image
		g_string_append_printf(tweet,
				" <a href=\"" TWITTER_URI ":///" TWITTER_URI_ACTION_ACTIONS "?account=a%s&user=%s&id=%lld",
				account_name,
				purple_url_encode(src_user),
				tweet_id);
		g_string_append_printf(tweet,
				"&conv_type=%d&conv_name=%s\">*</a>",
				conv_type,
				purple_url_encode(conv_name));
	}
#else

	if (twitter_option_add_link_to_tweet(account) && is_tweet && tweet_id)
	{
		g_string_append_printf(tweet,
				"\nhttp://twitter.com/%s/status/%lld\n",
				src_user,
				tweet_id);
	}
#endif

	g_free(linkified_message);
	return g_string_free(tweet, FALSE);
}


gchar *twitter_utf8_find_last_pos(const gchar *str, const gchar *needles, glong str_len)
{
	gchar *last;
	const gchar *needle;
	for (last = g_utf8_offset_to_pointer(str, str_len); last; last = g_utf8_find_prev_char(str, last))
		for (needle = needles; *needle; needle++)
			if (*last == *needle)
			{
				return last;
			}
	return NULL;
}

char *twitter_utf8_get_segment(const gchar *message, int max_len, const gchar *add_text, const gchar **new_start)
{
	int add_text_len = 0;
	int index_add_text = -1;
	char *status;
	int len_left;
	int len = 0;
	static const gchar *spaces = " \r\n";

	while (message[0] == ' ')
		message++;

	if (message[0] == '\0')
		return NULL;

	if (add_text)
	{
		gchar *message_lower = g_utf8_strdown(message, -1);
		char *pnt_add_text = strstr(message_lower, add_text);
		add_text_len = g_utf8_strlen(add_text, -1);
		if (pnt_add_text)
			index_add_text = g_utf8_pointer_to_offset(message, pnt_add_text) + add_text_len;
		g_free(message_lower);
	}

	len_left = g_utf8_strlen(message, -1);
	if (len_left <= max_len && (!add_text || index_add_text != -1))
	{
		status = g_strdup(message);
		len = strlen(message);
	} else if (len_left <= max_len && len_left + add_text_len + 1 <= max_len) {
		status = g_strdup_printf("%s %s", add_text, message);
		len = strlen(message);
	} else {
		gchar *space;
		if (add_text 
			&& index_add_text != -1
			&& index_add_text <= max_len
			&& (space = twitter_utf8_find_last_pos(message + index_add_text, spaces, max_len - g_utf8_pointer_to_offset(message, message + index_add_text)))
			&& (g_utf8_pointer_to_offset(message, space) <= max_len))
		{
			//split already has our word in it
			len = space - message;
			status = g_strndup(message, len);
			len++;
		} else if ((space = twitter_utf8_find_last_pos(message, spaces, max_len - (add_text ? add_text_len + 1 : 0)))) {
			len = space - message;
			space[0] = '\0';
			status = add_text ? g_strdup_printf("%s %s", add_text, message) : g_strdup(message);
			space[0] = ' ';
			len++;
		} else if (index_add_text != -1 && index_add_text <= max_len) {
			//one long word, which contains our add_text
			char prev_char;
			gchar *end_pos;
			end_pos = g_utf8_offset_to_pointer(message, max_len);
			len = end_pos - message;
			prev_char = end_pos[0];
			end_pos[0] = '\0';
			status = g_strdup(message);
			end_pos[0] = prev_char;
		} else {
			char prev_char;
			gchar *end_pos;
			end_pos = (index_add_text != -1 && index_add_text <= max_len ?
					g_utf8_offset_to_pointer(message, max_len) :
					g_utf8_offset_to_pointer(message, max_len - (add_text ? add_text_len + 1 : 0)));
			end_pos = g_utf8_offset_to_pointer(message, max_len - (add_text ? add_text_len + 1 : 0));
			len = end_pos - message;
			prev_char = end_pos[0];
			end_pos[0] = '\0';
			status = add_text ? g_strdup_printf("%s %s", add_text, message) : g_strdup(message);
			end_pos[0] = prev_char;
		}
	}
	if (new_start)
		*new_start = message + len;
	return g_strstrip(status);
}
GArray *twitter_utf8_get_segments(const gchar *message, int segment_length, const gchar *add_text)
{
	GArray *segments;
	const gchar *new_start = NULL;
	const gchar *pos;
	gchar *segment = twitter_utf8_get_segment(message, segment_length, add_text, &new_start);
	segments = g_array_new(FALSE, FALSE, sizeof(char *));
	while (segment)
	{
		g_array_append_val(segments, segment);
		pos = new_start;
		segment = twitter_utf8_get_segment(pos, segment_length, add_text, &new_start);
	};
	return segments;
}
