#include "twitter_chaticon.h"

/*
 * Conversation input characters count plugin.
 * Based on the Markerline plugin.
 *
 * Copyright (C) 2007 Dossy Shiobara <dossy@panoptic.com>
 * Copyright (C) 2010 Neaveru <neaveru@gmail.com>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02111-1301, USA.
 */

#if _HAVE_PIDGIN_

#include "twitter_conn.h"
#include "twitter_request.h"

typedef struct
{
	PurpleAccount *account;
	gchar *buddy_name;
	gchar *url;
} BuddyIconContext;
static void insert_requested_icon(TwitterConvIcon *data);

#define twitter_debug(fmt, ...)	purple_debug_info(TWITTER_PROTOCOL_ID, "%s: %s():%4d:  " fmt, __FILE__, __FUNCTION__, (int)__LINE__, ## __VA_ARGS__);

static BuddyIconContext *twitter_buddy_icon_context_new(PurpleAccount *account, const gchar *buddy_name, const gchar *url)
{
	BuddyIconContext *ctx = g_new0(BuddyIconContext, 1);
	ctx->account = account;
	ctx->buddy_name = g_strdup(buddy_name);
	ctx->url = g_strdup(url);
	return ctx;
}

static void twitter_buddy_icon_context_free(BuddyIconContext *ctx)
{
	if (!ctx)
		return;
	if (ctx->buddy_name)
		g_free(ctx->buddy_name);
	if (ctx->url)
		g_free(ctx->url);
	g_free(ctx);
}

static GdkPixbuf *make_scaled_pixbuf(const guchar *url_text, gsize len)
{
	/* make pixbuf */
	GdkPixbufLoader *loader;
	GdkPixbuf *src = NULL, *dest = NULL;
	gint size;

	g_return_val_if_fail(url_text != NULL, NULL);
	g_return_val_if_fail(len > 0, NULL);

	loader = gdk_pixbuf_loader_new();
	gdk_pixbuf_loader_write(loader, url_text, len, NULL);
	gdk_pixbuf_loader_close(loader, NULL);

	src = gdk_pixbuf_loader_get_pixbuf(loader);

	if(src)
	{
		size = 48;
		dest = gdk_pixbuf_scale_simple(src, size, size, GDK_INTERP_HYPER);
	} else {
		dest = NULL;
	}

	g_object_unref(G_OBJECT(loader));

	return dest;
}

static void conv_icon_clear(TwitterConvIcon *conv_icon)
{
	g_return_if_fail(conv_icon != NULL);

	if (conv_icon->icon_url)
		g_free(conv_icon->icon_url);
	conv_icon->icon_url = NULL;

	if (conv_icon->pixbuf)
		g_object_unref(G_OBJECT(conv_icon->pixbuf));
	conv_icon->pixbuf = NULL;
}

static void conv_icon_set_buddy_icon_data(TwitterConvIcon *conv_icon, PurpleBuddyIcon *buddy_icon)
{
	gsize len;
	const guchar *data;
	g_return_if_fail(conv_icon != NULL);

	conv_icon_clear(conv_icon);

	if (buddy_icon)
	{
		data = purple_buddy_icon_get_data(buddy_icon, &len);

		conv_icon->icon_url = g_strdup(purple_buddy_icon_get_checksum(buddy_icon));
		conv_icon->pixbuf = make_scaled_pixbuf(data, len);
	}

}
static TwitterConvIcon *buddy_icon_to_conv_icon(PurpleBuddyIcon *buddy_icon)
{
	TwitterConvIcon *conv_icon;

	g_return_val_if_fail(buddy_icon != NULL, NULL);

	conv_icon = g_new0(TwitterConvIcon, 1);
	conv_icon_set_buddy_icon_data(conv_icon, buddy_icon);

	return conv_icon;
}

void twitter_conv_icon_got_buddy_icon(PurpleAccount *account, const char *user_name, PurpleBuddyIcon *buddy_icon)
{
	PurpleConnection *gc = purple_account_get_connection(account);
	TwitterConnectionData *twitter = gc->proto_data;
	TwitterConvIcon *conv_icon = g_hash_table_lookup(twitter->icons, purple_normalize(account, user_name));

	if (!conv_icon)
	{
		return;
	}

	if (!conv_icon->request_list)
	{
		conv_icon_clear(conv_icon);
	} else {
		conv_icon_set_buddy_icon_data(conv_icon, buddy_icon);
	}

	if (conv_icon->pixbuf)
		insert_requested_icon(conv_icon);
}

static TwitterConvIcon *twitter_conv_icon_find(PurpleAccount *account, const char *who)
{
	PurpleConnection *gc = purple_account_get_connection(account);
	PurpleBuddyIcon *buddy_icon;
	TwitterConvIcon *conv_icon;
	TwitterConnectionData *twitter = gc->proto_data;

	twitter_debug("Looking up %s\n", who);
	conv_icon = g_hash_table_lookup(twitter->icons, purple_normalize(account, who));
	if ((!conv_icon || !conv_icon->pixbuf) && (buddy_icon = purple_buddy_icons_find(account, who)))
	{
		twitter_debug("Found buddy_icon\n");
		if (!conv_icon)
		{
			if ((conv_icon = buddy_icon_to_conv_icon(buddy_icon)))
				g_hash_table_insert(twitter->icons, g_strdup(purple_normalize(account, who)), conv_icon);
		}
		else
		{
			conv_icon_set_buddy_icon_data(conv_icon, buddy_icon);
		}
		purple_buddy_icon_unref(buddy_icon);
	}
	return conv_icon; //may be NULL
}


static void remove_marks_func(TwitterConvIcon *data, GtkTextBuffer *text_buffer)
{
	GList *mark_list = NULL;
	GList *current;

	if(!data)
		return;

	if(data->request_list)
		mark_list = data->request_list;

	/* remove the marks in its GtkTextBuffers */
	current = g_list_first(mark_list);
	while(current) {
		GtkTextMark *current_mark = current->data;
		GtkTextBuffer *current_text_buffer =
			gtk_text_mark_get_buffer(current_mark);
		GList *next;

		next = g_list_next(current);

		if(!current_text_buffer)
			continue;

		if(text_buffer) {
			if(current_text_buffer == text_buffer) {
				/* the mark will be freed in this function */
				gtk_text_buffer_delete_mark(current_text_buffer,
						current_mark);
				current->data = NULL;
				mark_list = g_list_delete_link(mark_list, current);
			}
		}
		else {
			gtk_text_buffer_delete_mark(current_text_buffer, current_mark);
			current->data = NULL;
			mark_list = g_list_delete_link(mark_list, current);
		}

		current = next;
	}

	data->request_list = mark_list;
}

static void insert_icon_at_mark(GtkTextMark *requested_mark, gpointer user_data)
{
	GList *win_list;
	GtkIMHtml *target_imhtml = NULL;
	GtkTextBuffer *target_buffer = NULL;
	GtkTextIter insertion_point;
	TwitterConvIcon *data = user_data;

	/* find the conversation that contains the mark  */
	for(win_list = pidgin_conv_windows_get_list(); win_list;
			win_list = win_list->next) {
		PidginWindow *win = win_list->data;
		GList *conv_list;

		for(conv_list = pidgin_conv_window_get_gtkconvs(win); conv_list;
				conv_list = conv_list->next) {
			PidginConversation *conv = conv_list->data;

			GtkIMHtml *current_imhtml = GTK_IMHTML(conv->imhtml);
			GtkTextBuffer *current_buffer = gtk_text_view_get_buffer(
					GTK_TEXT_VIEW(current_imhtml));

			if(current_buffer == gtk_text_mark_get_buffer(requested_mark)) {
				target_imhtml = current_imhtml;
				target_buffer = current_buffer;
				break;
			}
		}
	}

	if(!(target_imhtml && target_buffer)) {
		twitter_debug("No target imhtml/target buffer\n");
		return;
	}

	/* insert icon to the mark */
	gtk_text_buffer_get_iter_at_mark(target_buffer,
			&insertion_point, requested_mark);

	/* in this function, we put an icon for pending marks. we should
	 * not invalidate the icon here, otherwise it may result in
	 * thrashing. --yaz */

	if(!data || !data->pixbuf) {
		twitter_debug("No pixbuf\n");
		return;
	}

	/* insert icon actually */
	data->use_count++;
	gtk_text_buffer_insert_pixbuf(target_buffer,
			&insertion_point,
			data->pixbuf);

	gtk_text_buffer_delete_mark(target_buffer, requested_mark);
	requested_mark = NULL;
	twitter_debug("inserted\n");
}

static void insert_requested_icon(TwitterConvIcon *data)
{
	GList *mark_list = NULL;

	if(!data)
		return;

	mark_list = data->request_list;

	twitter_debug("about to insert icon for pending requests\n");

	if(mark_list) {
		g_list_foreach(mark_list, (GFunc) insert_icon_at_mark, data);
		mark_list = g_list_remove_all(mark_list, NULL);
		g_list_free(mark_list);
		data->request_list = NULL;
	}

}


static void got_page_cb(PurpleUtilFetchUrlData *url_data, gpointer user_data,
		            const gchar *url_text, gsize len, const gchar *error_message)
{
	BuddyIconContext *ctx = user_data;
	TwitterConvIcon *conv_icon;
	const gchar *pic_data;

	conv_icon = twitter_conv_icon_find(ctx->account, ctx->buddy_name);
	twitter_buddy_icon_context_free(ctx);

	g_return_if_fail(conv_icon != NULL);

	conv_icon->requested = FALSE;
	conv_icon->fetch_data = NULL;

	if (len && !error_message && twitter_response_text_status_code(url_text) == 200 && (pic_data = twitter_response_text_data(url_text, len)))
	{
		twitter_debug("Attempting to create pixbuf\n");
		conv_icon->pixbuf = make_scaled_pixbuf((const guchar *) pic_data, len);
	}

	if (conv_icon->pixbuf)
	{
		twitter_debug("All succeeded, inserting\n");
		insert_requested_icon(conv_icon);
	}
}

void twitter_request_conv_icon(PurpleAccount *account, const char *user_name, const gchar *url, time_t icon_time)
{
	/* look local icon cache for the requested icon */
	PurpleConnection *gc = purple_account_get_connection(account);
	TwitterConnectionData *twitter = gc->proto_data;
	TwitterConvIcon *data = NULL;
	GHashTable *hash = twitter->icons;

	if(!hash)
		return;

	/* since this function is called after mark_icon_for_user(), data
	 * must exist here. */
	data = twitter_conv_icon_find(account, user_name);
	if (!data)
	{
		data = g_new0(TwitterConvIcon, 1);
		g_hash_table_insert(hash, g_strdup(purple_normalize(account, user_name)), data);
		data->mtime = icon_time;
	} else {
		//A new icon is one posted with a tweet later than the current saved icon time
		//and with a different url
		gboolean new_icon = strcmp(url, data->icon_url) && icon_time > data->mtime;

		twitter_debug("Have icon %s (%lld) for user %s, looking for %s (%lld)\n",
			data->icon_url, (long long int) data->mtime, user_name,
			url, (long long int) icon_time);

		if (icon_time > data->mtime)
			data->mtime = icon_time;

		//Return if the image is cached already and it's the same one
		if (data->pixbuf && !new_icon)
			return;

		/* Return if user's icon has been requested already. */
		if (data->requested && !new_icon)
			return;

		//If we're already requesting, but it's a different url, cancel the fetch
		if (data->fetch_data)
			purple_util_fetch_url_cancel(data->fetch_data);

		conv_icon_clear(data);
	}

	data->icon_url = g_strdup(url);
	
	//For buddies, we don't want to retrieve the icon here, we'll
	//let the twitter_buddy fetch the icon and let us know when it's done
	if (purple_find_buddy(account, user_name))
		return;

	data->requested = TRUE; 

	/* Create the URL for an user's icon. */
	if(url) {
		BuddyIconContext *ctx = twitter_buddy_icon_context_new(account, user_name, url);
		twitter_debug("requesting %s\n", url);
		data->fetch_data =
			purple_util_fetch_url_request(url, TRUE, NULL, FALSE, NULL, TRUE, got_page_cb, ctx);
	}
}

void twitter_conv_icon_free(TwitterConvIcon *conv_icon)
{
	if (!conv_icon)
		return;
	if (conv_icon->requested)
	{
		purple_util_fetch_url_cancel(conv_icon->fetch_data);
		conv_icon->fetch_data = NULL;
		conv_icon->requested = FALSE;
	}
	if (conv_icon->request_list)
		remove_marks_func(conv_icon, NULL);
	conv_icon->request_list = NULL;

	if (conv_icon->pixbuf)
	{
		g_object_unref(G_OBJECT(conv_icon->pixbuf));
	}
	conv_icon->pixbuf = NULL;

	if (conv_icon->icon_url)
		g_free(conv_icon->icon_url);
	conv_icon->icon_url = NULL;

	g_free(conv_icon);
}

static void mark_icon_for_user(GtkTextMark *mark, TwitterConvIcon *data)
{
	data->request_list = g_list_prepend(data->request_list, mark);
}

static gboolean twitter_chat_icon_displaying_chat_cb(PurpleAccount *account, const char *who, char **message,
		PurpleConversation *conv, PurpleMessageFlags flags,
		void *unused)
{
	GtkIMHtml *imhtml;
	GtkTextBuffer *text_buffer;
	gint linenumber = 0;

	twitter_debug("called\n");

	/* get text buffer */
	imhtml = GTK_IMHTML(PIDGIN_CONVERSATION(conv)->imhtml);
	text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(imhtml));

	/* store number of lines */
	linenumber = gtk_text_buffer_get_line_count(text_buffer);

	//this is just used to pass the current line number (before the insertion of the new text)
	//to the _displayed_chat_cb func. 
	//Since pidgin is (mostly) single threaded, this is probably overkill and we could just use
	//a single global int. 
	//On another note, we don't insert the icon here because the message may not end up being displayed
	//based on what other plugins do
	purple_conversation_set_data(conv, TWITTER_PROTOCOL_ID "-icon-ln", GINT_TO_POINTER(linenumber));
	twitter_debug("conv = %p linenumber = %d\n", conv, linenumber);

	return FALSE;
}


static void twitter_chat_icon_displayed_chat_cb(PurpleAccount *account, const char *who, char *message,
		PurpleConversation *conv, PurpleMessageFlags flags)
{
	GtkIMHtml *imhtml;
	GtkTextBuffer *text_buffer;
	GtkTextIter insertion_point;
	gint linenumber;
	TwitterConvIcon *conv_icon;
	PurpleConnection *gc = purple_account_get_connection(account);
	TwitterConnectionData *twitter = gc->proto_data;

	twitter_debug("called\n");

	/* insert icon */
	imhtml = GTK_IMHTML(PIDGIN_CONVERSATION(conv)->imhtml);
	text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(imhtml));

	/* get GtkTextIter in the target line */
	linenumber = GPOINTER_TO_INT(purple_conversation_get_data(conv, TWITTER_PROTOCOL_ID "-icon-ln"));
	gtk_text_buffer_get_iter_at_line(text_buffer,
			&insertion_point,
			linenumber);

	conv_icon = twitter_conv_icon_find(account, who);

	/* if we don't have the icon for this user, put a mark instead and
	 * request the icon */
	if (!conv_icon) {
		conv_icon = g_new0(TwitterConvIcon, 1);
		g_hash_table_insert(twitter->icons, g_strdup(purple_normalize(account, who)), conv_icon);
		mark_icon_for_user(gtk_text_buffer_create_mark(text_buffer, NULL, &insertion_point, FALSE),
				conv_icon);
		return;
	}

	/* if we have the icon for this user, insert the icon immediately */
	if (TRUE)
	{
		if (conv_icon->pixbuf)
		{
			gtk_text_buffer_insert_pixbuf(text_buffer,
					&insertion_point,
					conv_icon->pixbuf);
		} else {
			mark_icon_for_user(gtk_text_buffer_create_mark(
						text_buffer, NULL, &insertion_point, FALSE), conv_icon);
		}
	}

	twitter_debug("reach end of function\n");
}

void twitter_chat_icon_init(PurplePlugin *plugin)
{
	twitter_debug("Init\n");
	purple_signal_connect(pidgin_conversations_get_handle(),
			"displaying-chat-msg",
			plugin, PURPLE_CALLBACK(twitter_chat_icon_displaying_chat_cb), NULL);
	purple_signal_connect(pidgin_conversations_get_handle(),
			"displayed-chat-msg",
			plugin, PURPLE_CALLBACK(twitter_chat_icon_displayed_chat_cb), NULL);
}
#endif
