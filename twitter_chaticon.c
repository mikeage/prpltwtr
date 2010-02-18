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

#define twitter_debug(fmt, ...)	purple_debug_info(TWITTER_PROTOCOL_ID, "%s: %s():%4d:  " fmt, __FILE__, __FUNCTION__, (int)__LINE__, ## __VA_ARGS__);

static GHashTable *conv_hash = NULL;

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
	g_hash_table_insert(conv_hash, conv, GINT_TO_POINTER(linenumber));
	twitter_debug("conv = %p linenumber = %d\n", conv, linenumber);

	return FALSE;
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

static void twitter_chat_icon_displayed_chat_cb(PurpleAccount *account, const char *who, char *message,
		PurpleConversation *conv, PurpleMessageFlags flags)
{
	GtkIMHtml *imhtml;
	GtkTextBuffer *text_buffer;
	GtkTextIter insertion_point;
	gint linenumber;
	GdkPixbuf *icon_pixbuf;
	PurpleBuddyIcon *buddy_icon;

	twitter_debug("called\n");

	/* insert icon */
	imhtml = GTK_IMHTML(PIDGIN_CONVERSATION(conv)->imhtml);
	text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(imhtml));

	/* get GtkTextIter in the target line */
	linenumber = GPOINTER_TO_INT(g_hash_table_lookup(conv_hash, conv));
	gtk_text_buffer_get_iter_at_line(text_buffer,
			&insertion_point,
			linenumber);

	buddy_icon = purple_buddy_icons_find(account, who);

	/* if we don't have the icon for this user, put a mark instead and
	 * request the icon */
	if (!buddy_icon) {
		twitter_debug("%s's icon is not in memory.\n", who);
		/* request to attach icon to the buffer */
		//XXX
		//request_icon(user_name, service, renew);
		return;
	}

	/* if we have the icon for this user, insert the icon immediately */
	if (TRUE)
	{
		const guchar *data;
		gsize len;
		data = purple_buddy_icon_get_data(buddy_icon, &len);
		icon_pixbuf = make_scaled_pixbuf(data, len);
		if (icon_pixbuf)
		{
			gtk_text_buffer_insert_pixbuf(text_buffer,
					&insertion_point,
					icon_pixbuf);
			g_object_unref(G_OBJECT(icon_pixbuf));
		}
	}
	purple_buddy_icon_unref(buddy_icon);

	twitter_debug("reach end of function\n");
}


void twitter_chat_icon_init(PurplePlugin *plugin)
{
	twitter_debug("Init\n");
	conv_hash = g_hash_table_new_full(g_direct_hash, g_direct_equal,
			NULL, NULL);
	purple_signal_connect(pidgin_conversations_get_handle(),
			"displaying-chat-msg",
			plugin, PURPLE_CALLBACK(twitter_chat_icon_displaying_chat_cb), NULL);
	purple_signal_connect(pidgin_conversations_get_handle(),
			"displayed-chat-msg",
			plugin, PURPLE_CALLBACK(twitter_chat_icon_displayed_chat_cb), NULL);
}
#endif
