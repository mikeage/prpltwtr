#include "gtkprpltwtr_charcount.h"

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

#include "string.h"
#include <gdk/gdk.h>
#include <glib.h>
#include <gtk/gtk.h>

#include <gtkconv.h>
#include <gtkimhtml.h>
#include <gtkplugin.h>
#include <version.h>

#include "prpltwtr_endpoint_chat.h"
#include "prpltwtr_endpoint_im.h"
#include "gtkprpltwtr.h"

/* data to get passed each time a character is typed
 * this should be faster than having multiple hash
 * table lookups everytime we type a char */
typedef struct
{
	PidginConversation *gtkconv;
	gchar *append_text;
	int append_text_len;
} ConvCharCount;

//Don't forget to free
static gchar *twitter_conv_get_append_text(PurpleConversation *conv)
{
	if (conv->type == PURPLE_CONV_TYPE_CHAT)
	{
		TwitterEndpointChat *ctx = twitter_endpoint_chat_find(purple_conversation_get_account(conv), purple_conversation_get_name(conv));
		if (ctx && ctx->settings->get_status_added_text)
		{
			PurpleChat *chat = twitter_blist_chat_find(purple_conversation_get_account(conv), ctx->chat_name);
			if (chat && TWITTER_ATTACH_SEARCH_TEXT_NONE != twitter_blist_chat_attach_search_text(chat))
			{
				return ctx->settings->get_status_added_text(ctx);
			}
		}
	} else if (conv->type == PURPLE_CONV_TYPE_IM) {
		PurpleAccount *account = purple_conversation_get_account(conv);
		const gchar *conv_name = purple_conversation_get_name(conv);
		if (twitter_conv_name_to_type(account, conv_name) == TWITTER_IM_TYPE_AT_MSG)
		{
			return g_strdup_printf("@%s", twitter_conv_name_to_buddy_name(account, conv_name));
		}
	}
	return NULL;
}

static ConvCharCount *conv_char_count_new(PidginConversation *gtkconv)
{
	ConvCharCount *ccc;
	gchar *append_text;
	g_return_val_if_fail(gtkconv != NULL, NULL);

	ccc = g_new(ConvCharCount, 1);
	ccc->gtkconv = gtkconv;
	append_text = twitter_conv_get_append_text(gtkconv->active_conv);
	ccc->append_text = append_text ? g_utf8_strdown(append_text, -1) : NULL; 
	ccc->append_text_len = ccc->append_text ? g_utf8_strlen(ccc->append_text, -1) + 1 : 0;
	if (append_text)
		g_free(append_text);
	return ccc;
}
static void conv_char_count_free(ConvCharCount *ccc)
{
	if (!ccc)
		return;
	if (ccc->append_text)
		g_free(ccc->append_text);
	g_free(ccc);
}

static void changed_cb(GtkTextBuffer *textbuffer, gpointer user_data)
{
	ConvCharCount *ccc = (ConvCharCount *) user_data;
	PidginConversation *gtkconv;
	GtkWidget *box, *counter = NULL;
	gchar count[20];

	static GtkTextIter start_iter, end_iter;
	gchar *text, *text_lower;
	int append_text_len;

	g_return_if_fail(ccc != NULL);

	gtkconv = ccc->gtkconv;
	append_text_len = ccc->append_text_len;

	gtk_text_buffer_get_start_iter(textbuffer, &start_iter);
	gtk_text_buffer_get_end_iter(textbuffer, &end_iter);

	if (ccc->append_text)
	{
		text = gtk_text_buffer_get_text(textbuffer, &start_iter, &end_iter, TRUE);
		text_lower = g_utf8_strdown(text, -1);
		if (strstr(text_lower, ccc->append_text))
		{
			append_text_len = 0;
		}
		g_free(text);
		g_free(text_lower);
	}

	g_snprintf(count, sizeof(count) - 1, "%u",
			gtk_text_buffer_get_char_count(textbuffer) +
			append_text_len);

	box = gtkconv->toolbar;
	counter = g_object_get_data(G_OBJECT(box), TWITTER_PROTOCOL_ID "-counter");
	if (counter)
		gtk_label_set_text(GTK_LABEL(counter), count);
}

/*
static void insert_text_cb(GtkTextBuffer *textbuffer, GtkTextIter *position, gchar *new_text, gint new_text_length, gpointer user_data)
{
	ConvCharCount *ccc = (ConvCharCount *) user_data;
	PidginConversation *gtkconv;
	GtkWidget *box, *counter = NULL;
	gchar count[20];

	static GtkTextIter start_iter, end_iter;

	g_return_if_fail(ccc != NULL);

	gtkconv = ccc->gtkconv;

	gtk_text_buffer_get_start_iter(textbuffer, &start_iter);
	gtk_text_buffer_get_end_iter(textbuffer, &end_iter);
	gchar *text = gtk_text_buffer_get_text(textbuffer, &start_iter, &end_iter, TRUE);
	printf("%s\n", text);
	g_free(text);
	g_snprintf(count, sizeof(count) - 1, "%u",
			gtk_text_buffer_get_char_count(textbuffer) +
			new_text_length + ccc->append_text_len);

	box = gtkconv->toolbar;
	counter = g_object_get_data(G_OBJECT(box), TWITTER_PROTOCOL_ID "-counter");
	if (counter)
		gtk_label_set_text(GTK_LABEL(counter), count);
}
*/

/*
static void delete_text_cb(GtkTextBuffer *textbuffer, GtkTextIter *start_pos, GtkTextIter *end_pos, gpointer user_data)
{
	ConvCharCount *ccc = (ConvCharCount *) user_data;
	PidginConversation *gtkconv;
	GtkWidget *box, *counter = NULL;
	gchar count[20];

	g_return_if_fail(ccc != NULL);

	gtkconv = ccc->gtkconv;

	g_snprintf(count, sizeof(count) - 1, "%u",
			gtk_text_buffer_get_char_count(textbuffer) + ccc->append_text_len -
			(gtk_text_iter_get_offset(end_pos) - gtk_text_iter_get_offset(start_pos)));

	box = gtkconv->toolbar;
	counter = g_object_get_data(G_OBJECT(box), TWITTER_PROTOCOL_ID "-counter");
	if (counter)
		gtk_label_set_text(GTK_LABEL(counter), count);
}
*/

static void detach_from_gtkconv(PidginConversation *gtkconv, gpointer null)
{
	ConvCharCount *ccc;
	GtkWidget *box, *counter = NULL, *sep = NULL;

	PurpleAccount *account = purple_conversation_get_account(gtkconv->active_conv);
	if (strcmp(TWITTER_PROTOCOL_ID, purple_account_get_protocol_id(account)) && strcmp(STATUSNET_PROTOCOL_ID, purple_account_get_protocol_id(account)))
		return;

	/*g_signal_handlers_disconnect_by_func(G_OBJECT(gtkconv->entry_buffer),
			(GFunc)insert_text_cb, gtkconv);
	g_signal_handlers_disconnect_by_func(G_OBJECT(gtkconv->entry_buffer),
			(GFunc)delete_text_cb, gtkconv);*/

	box = gtkconv->toolbar;
	counter = g_object_steal_data(G_OBJECT(box), TWITTER_PROTOCOL_ID "-counter");
	if (counter)
		gtk_container_remove(GTK_CONTAINER(box), counter);
	sep = g_object_steal_data(G_OBJECT(box), TWITTER_PROTOCOL_ID "-sep");
	if (sep)
		gtk_container_remove(GTK_CONTAINER(box), sep);

	ccc = g_object_steal_data(G_OBJECT(box), TWITTER_PROTOCOL_ID "-ccc");
	g_signal_handlers_disconnect_by_func(G_OBJECT(gtkconv->entry_buffer),
			G_CALLBACK(changed_cb), ccc);
	if (ccc)
	{
		conv_char_count_free(ccc);
	}

	gtk_widget_queue_draw(pidgin_conv_get_window(gtkconv)->window);
}


static void attach_to_gtkconv(PidginConversation *gtkconv, gpointer null)
{
	GtkWidget *box, *sep, *counter;
	ConvCharCount *ccc;
	gchar count[20];
	PurpleAccount *account;

	account = purple_conversation_get_account(gtkconv->active_conv);
	if (strcmp(TWITTER_PROTOCOL_ID, purple_account_get_protocol_id(account)) && strcmp(STATUSNET_PROTOCOL_ID, purple_account_get_protocol_id(account)))
		return;

	ccc = conv_char_count_new(gtkconv);

	g_snprintf(count, sizeof(count) - 1, "%u", ccc->append_text_len);

	box = gtkconv->toolbar;
	counter = g_object_get_data(G_OBJECT(box), TWITTER_PROTOCOL_ID "-counter");
	g_return_if_fail(counter == NULL);

	counter = gtk_label_new(NULL);
	gtk_widget_set_name(counter, "convcharcount_label");
	gtk_label_set_text(GTK_LABEL(counter), count);
	gtk_box_pack_end(GTK_BOX(box), counter, FALSE, FALSE, 0);
	gtk_widget_show_all(counter);

	g_object_set_data(G_OBJECT(box), TWITTER_PROTOCOL_ID "-counter", counter);

	sep = gtk_vseparator_new();
	gtk_box_pack_end(GTK_BOX(box), sep, FALSE, FALSE, 0);
	gtk_widget_show_all(sep);

	g_object_set_data(G_OBJECT(box), TWITTER_PROTOCOL_ID "-sep", sep);

	g_object_set_data(G_OBJECT(box), TWITTER_PROTOCOL_ID "-ccc", ccc);

	/* connect signals, etc. */
	/*g_signal_connect(G_OBJECT(gtkconv->entry_buffer), "insert_text",
			G_CALLBACK(insert_text_cb), ccc);
	g_signal_connect(G_OBJECT(gtkconv->entry_buffer), "delete_range",
			G_CALLBACK(delete_text_cb), ccc);*/
	g_signal_connect(G_OBJECT(gtkconv->entry_buffer), "changed",
			G_CALLBACK(changed_cb), ccc);

	//Call right away, in case there's already text in the buffer
	changed_cb(gtkconv->entry_buffer, ccc);

	gtk_widget_queue_draw(pidgin_conv_get_window(gtkconv)->window);
}

void twitter_charcount_update_append_text_cb(PurpleConversation *conv)
{
	PidginConversation * gtkconv = PIDGIN_CONVERSATION(conv);
	ConvCharCount *ccc;
	gchar *append_text;

	ccc = g_object_get_data(G_OBJECT(gtkconv->toolbar), TWITTER_PROTOCOL_ID "-ccc");

	append_text = twitter_conv_get_append_text(gtkconv->active_conv);
	ccc->append_text = append_text ? g_utf8_strdown(append_text, -1) : NULL; 
	ccc->append_text_len = ccc->append_text ? g_utf8_strlen(ccc->append_text, -1) + 1 : 0;
	if (append_text)
		g_free(append_text);

	changed_cb(gtkconv->entry_buffer, ccc);
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
	PidginConversation *gtkconv;

	gtkconv = PIDGIN_CONVERSATION(conv);

	g_return_if_fail(gtkconv != NULL);

	attach_to_gtkconv(gtkconv, NULL);
}

void twitter_charcount_conv_destroyed_cb(PurpleConversation *conv, gpointer null)
{
	PidginConversation *gtkconv;

	gtkconv = PIDGIN_CONVERSATION(conv);

	g_return_if_fail(gtkconv != NULL);

	detach_from_gtkconv(gtkconv, NULL);
}
