#include "twitter_charcount.h"

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

#include "config.h"
#include "string.h"
#include <gdk/gdk.h>
#include <glib.h>
#include <gtk/gtk.h>

#include <gtkconv.h>
#include <gtkimhtml.h>
#include <gtkplugin.h>
#include <version.h>

static void insert_text_cb(GtkTextBuffer *textbuffer, GtkTextIter *position, gchar *new_text, gint new_text_length, gpointer user_data)
{
	PidginConversation *gtkconv = (PidginConversation *)user_data;
	GtkWidget *box, *counter = NULL;
	gchar count[20];

	g_return_if_fail(gtkconv != NULL);

	g_snprintf(count, sizeof(count) - 1, "%u",
			gtk_text_buffer_get_char_count(textbuffer) +
			new_text_length);

	box = gtkconv->toolbar;
	counter = g_object_get_data(G_OBJECT(box), TWITTER_PROTOCOL_ID "-counter");
	if (counter)
		gtk_label_set_text(GTK_LABEL(counter), count);
}

static void delete_text_cb(GtkTextBuffer *textbuffer, GtkTextIter *start_pos, GtkTextIter *end_pos, gpointer user_data)
{
	PidginConversation *gtkconv = (PidginConversation *)user_data;
	GtkWidget *box, *counter = NULL;
	gchar count[20];

	g_return_if_fail(gtkconv != NULL);

	g_snprintf(count, sizeof(count) - 1, "%u",
			gtk_text_buffer_get_char_count(textbuffer) -
			(gtk_text_iter_get_offset(end_pos) - gtk_text_iter_get_offset(start_pos)));

	box = gtkconv->toolbar;
	counter = g_object_get_data(G_OBJECT(box), TWITTER_PROTOCOL_ID "-counter");
	if (counter)
		gtk_label_set_text(GTK_LABEL(counter), count);
}

static void detach_from_gtkconv(PidginConversation *gtkconv, gpointer null)
{
	GtkWidget *box, *counter = NULL, *sep = NULL;

	PurpleAccount *account = purple_conversation_get_account(gtkconv->active_conv);
	if (strcmp(purple_account_get_protocol_id(account), TWITTER_PROTOCOL_ID))
		return;

	g_signal_handlers_disconnect_by_func(G_OBJECT(gtkconv->entry_buffer),
			(GFunc)insert_text_cb, gtkconv);
	g_signal_handlers_disconnect_by_func(G_OBJECT(gtkconv->entry_buffer),
			(GFunc)delete_text_cb, gtkconv);

	box = gtkconv->toolbar;
	counter = g_object_get_data(G_OBJECT(box), TWITTER_PROTOCOL_ID "-counter");
	if (counter)
		gtk_container_remove(GTK_CONTAINER(box), counter);
	sep = g_object_get_data(G_OBJECT(box), TWITTER_PROTOCOL_ID "-sep");
	if (sep)
		gtk_container_remove(GTK_CONTAINER(box), sep);

	gtk_widget_queue_draw(pidgin_conv_get_window(gtkconv)->window);
}

static void attach_to_gtkconv(PidginConversation *gtkconv, gpointer null)
{
	GtkWidget *box, *sep, *counter;

	PurpleAccount *account = purple_conversation_get_account(gtkconv->active_conv);
	if (strcmp(purple_account_get_protocol_id(account), TWITTER_PROTOCOL_ID))
		return;

	box = gtkconv->toolbar;
	counter = g_object_get_data(G_OBJECT(box), TWITTER_PROTOCOL_ID "-counter");
	g_return_if_fail(counter == NULL);

	counter = gtk_label_new(NULL);
	gtk_widget_set_name(counter, "convcharcount_label");
	gtk_label_set_text(GTK_LABEL(counter), "0");
	gtk_box_pack_end(GTK_BOX(box), counter, FALSE, FALSE, 0);
	gtk_widget_show_all(counter);

	g_object_set_data(G_OBJECT(box), TWITTER_PROTOCOL_ID "-counter", counter);

	sep = gtk_vseparator_new();
	gtk_box_pack_end(GTK_BOX(box), sep, FALSE, FALSE, 0);
	gtk_widget_show_all(sep);

	g_object_set_data(G_OBJECT(box), TWITTER_PROTOCOL_ID "-sep", sep);

	/* connect signals, etc. */
	g_signal_connect(G_OBJECT(gtkconv->entry_buffer), "insert_text",
			G_CALLBACK(insert_text_cb), gtkconv);
	g_signal_connect(G_OBJECT(gtkconv->entry_buffer), "delete_range",
			G_CALLBACK(delete_text_cb), gtkconv);

	gtk_widget_queue_draw(pidgin_conv_get_window(gtkconv)->window);
}

static void detach_from_pidgin_window(PidginWindow *win, gpointer null)
{
	g_list_foreach(pidgin_conv_window_get_gtkconvs(win), (GFunc)detach_from_gtkconv, NULL);
}

static void attach_to_pidgin_window(PidginWindow *win, gpointer null)
{
	g_list_foreach(pidgin_conv_window_get_gtkconvs(win), (GFunc)attach_to_gtkconv, NULL);
}

void twitter_charcount_detach_from_all_windows()
{
	g_list_foreach(pidgin_conv_windows_get_list(), (GFunc)detach_from_pidgin_window, NULL);
}

void twitter_charcount_attach_to_all_windows()
{
	g_list_foreach(pidgin_conv_windows_get_list(), (GFunc)attach_to_pidgin_window, NULL);
}

void twitter_charcount_conv_created_cb(PurpleConversation *conv, gpointer null)
{
	PidginConversation *gtkconv = PIDGIN_CONVERSATION(conv);

	g_return_if_fail(gtkconv != NULL);

	attach_to_gtkconv(gtkconv, NULL);
}

#endif
