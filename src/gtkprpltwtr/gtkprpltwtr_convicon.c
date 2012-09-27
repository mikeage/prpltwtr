#include "gtkprpltwtr_convicon.h"

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
#include "prpltwtr_conn.h"
#include "prpltwtr_request.h"
#include "gtkprpltwtr.h"
#include "gtkprpltwtr_prefs.h"

typedef struct {
    PurpleAccount  *account;
    gchar          *buddy_name;
    gchar          *url;
} BuddyIconContext;
static void     insert_requested_icon(TwitterConvIcon * conv_icon);

static BuddyIconContext *twitter_buddy_icon_context_new(PurpleAccount * account, const gchar * buddy_name, const gchar * url)
{
    BuddyIconContext *ctx = g_new0(BuddyIconContext, 1);
    ctx->account = account;
    ctx->buddy_name = g_strdup(purple_normalize(account, buddy_name));
    ctx->url = g_strdup(url);
    return ctx;
}

static void twitter_buddy_icon_context_free(BuddyIconContext * ctx)
{
    if (!ctx)
        return;
    if (ctx->buddy_name)
        g_free(ctx->buddy_name);
    if (ctx->url)
        g_free(ctx->url);
    g_free(ctx);
}

static TwitterConvIcon *twitter_conv_icon_new(PurpleAccount * account, const gchar * username)
{
    TwitterConvIcon *conv_icon = g_new0(TwitterConvIcon, 1);
    conv_icon->username = g_strdup(purple_normalize(account, username));
    purple_debug_info(PLUGIN_ID, "Created conv icon %s\n", conv_icon->username);
    return conv_icon;
}

static GdkPixbuf *make_scaled_pixbuf(const guchar * url_text, gsize len)
{
    /* make pixbuf */
    GdkPixbufLoader *loader;
    GdkPixbuf      *src = NULL,
        *dest = NULL;

    g_return_val_if_fail(url_text != NULL, NULL);
    g_return_val_if_fail(len > 0, NULL);

    loader = gdk_pixbuf_loader_new();
    gdk_pixbuf_loader_write(loader, url_text, len, NULL);
    gdk_pixbuf_loader_close(loader, NULL);

    src = gdk_pixbuf_loader_get_pixbuf(loader);

    if (src) {
        dest = gdk_pixbuf_scale_simple(src, purple_prefs_get_int(TWITTER_PREF_CONV_ICON_SIZE), purple_prefs_get_int(TWITTER_PREF_CONV_ICON_SIZE), GDK_INTERP_HYPER);
    } else {
        dest = NULL;
    }

    g_object_unref(G_OBJECT(loader));

    return dest;
}

static void conv_icon_clear(TwitterConvIcon * conv_icon)
{
    g_return_if_fail(conv_icon != NULL);

    if (conv_icon->icon_url)
        g_free(conv_icon->icon_url);
    conv_icon->icon_url = NULL;

    if (conv_icon->pixbuf)
        g_object_unref(G_OBJECT(conv_icon->pixbuf));
    conv_icon->pixbuf = NULL;
}

static void conv_icon_set_buddy_icon_data(TwitterConvIcon * conv_icon, PurpleBuddyIcon * buddy_icon)
{
    gsize           len;
    const guchar   *data;
    g_return_if_fail(conv_icon != NULL);

    conv_icon_clear(conv_icon);

    if (buddy_icon) {
        data = purple_buddy_icon_get_data(buddy_icon, &len);

        conv_icon->icon_url = g_strdup(purple_buddy_icon_get_checksum(buddy_icon));
        conv_icon->pixbuf = make_scaled_pixbuf(data, len);
    }

}

static TwitterConvIcon *buddy_icon_to_conv_icon(PurpleBuddyIcon * buddy_icon)
{
    TwitterConvIcon *conv_icon;

    g_return_val_if_fail(buddy_icon != NULL, NULL);

    conv_icon = twitter_conv_icon_new(purple_buddy_icon_get_account(buddy_icon), purple_buddy_icon_get_username(buddy_icon));
    conv_icon_set_buddy_icon_data(conv_icon, buddy_icon);

    return conv_icon;
}

void twitter_conv_icon_got_buddy_icon(PurpleAccount * account, const char *user_name, PurpleBuddyIcon * buddy_icon)
{
    PurpleConnection *gc = purple_account_get_connection(account);
    TwitterConnectionData *twitter;
    TwitterConvIcon *conv_icon;

    if (!gc) {
        return;
    }

    twitter = gc->proto_data;

    if (!twitter) {
        return;
    }
    conv_icon = g_hash_table_lookup(twitter->icons, purple_normalize(account, user_name));

    if (!conv_icon) {
        return;
    }

    if (!conv_icon->request_list) {
        conv_icon_clear(conv_icon);
    } else {
        conv_icon_set_buddy_icon_data(conv_icon, buddy_icon);
    }

    if (conv_icon->pixbuf)
        insert_requested_icon(conv_icon);
}

static TwitterConvIcon *twitter_conv_icon_find(PurpleAccount * account, const char *who)
{
    PurpleConnection *gc = purple_account_get_connection(account);
    PurpleBuddyIcon *buddy_icon;
    TwitterConvIcon *conv_icon;
    TwitterConnectionData *twitter;

    if (!gc) {
        return NULL;
    }
    twitter = gc->proto_data;

    if (!twitter) {
        return NULL;
    }

    purple_debug_info(PLUGIN_ID, "Looking up %s\n", who);
    conv_icon = g_hash_table_lookup(twitter->icons, purple_normalize(account, who));
    if ((!conv_icon || !conv_icon->pixbuf) && (buddy_icon = purple_buddy_icons_find(account, who))) {
        if (!conv_icon) {
            if ((conv_icon = buddy_icon_to_conv_icon(buddy_icon)))
                g_hash_table_insert(twitter->icons, g_strdup(purple_normalize(account, who)), conv_icon);
        } else {
            conv_icon_set_buddy_icon_data(conv_icon, buddy_icon);
        }
        purple_buddy_icon_unref(buddy_icon);
    }
    return conv_icon;                            //may be NULL
}

static void remove_marks_func(TwitterConvIcon * conv_icon, GtkTextBuffer * text_buffer)
{
    GList          *mark_list = NULL;
    GList          *current;

    if (!conv_icon)
        return;

    if (conv_icon->request_list)
        mark_list = conv_icon->request_list;

    /* remove the marks in its GtkTextBuffers */
    current = g_list_first(mark_list);
    while (current) {
        GtkTextMark    *current_mark = current->data;
        GtkTextBuffer  *current_text_buffer = gtk_text_mark_get_buffer(current_mark);
        GList          *next;

        next = g_list_next(current);

        if (!current_text_buffer)
            continue;

        if (text_buffer) {
            if (current_text_buffer == text_buffer) {
                /* the mark will be freed in this function */
                gtk_text_buffer_delete_mark(current_text_buffer, current_mark);
                current->data = NULL;
                mark_list = g_list_delete_link(mark_list, current);
            }
        } else {
            gtk_text_buffer_delete_mark(current_text_buffer, current_mark);
            current->data = NULL;
            mark_list = g_list_delete_link(mark_list, current);
        }

        current = next;
    }

    conv_icon->request_list = mark_list;
}

static void insert_icon_at_mark(GtkTextMark * requested_mark, gpointer user_data)
{
    GList          *win_list;
    GtkIMHtml      *target_imhtml = NULL;
    GtkTextBuffer  *target_buffer = NULL;
    GtkTextIter     insertion_point;
    TwitterConvIcon *conv_icon = user_data;

    /* find the conversation that contains the mark  */
    for (win_list = pidgin_conv_windows_get_list(); win_list; win_list = win_list->next) {
        PidginWindow   *win = win_list->data;
        GList          *conv_list;

        for (conv_list = pidgin_conv_window_get_gtkconvs(win); conv_list; conv_list = conv_list->next) {
            PidginConversation *conv = conv_list->data;

            GtkIMHtml      *current_imhtml = GTK_IMHTML(conv->imhtml);
            GtkTextBuffer  *current_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(current_imhtml));

            if (current_buffer == gtk_text_mark_get_buffer(requested_mark)) {
                target_imhtml = current_imhtml;
                target_buffer = current_buffer;
                break;
            }
        }
    }

    if (!(target_imhtml && target_buffer)) {
        purple_debug_warning(PLUGIN_ID, "No target imhtml/target buffer\n");
        return;
    }

    /* insert icon to the mark */
    gtk_text_buffer_get_iter_at_mark(target_buffer, &insertion_point, requested_mark);

    /* in this function, we put an icon for pending marks. we should
     * not invalidate the icon here, otherwise it may result in
     * thrashing. --yaz */

    if (!conv_icon || !conv_icon->pixbuf) {
        purple_debug_warning(PLUGIN_ID, "No pixbuf\n");
        return;
    }

/* We only want to add the icon if the mark is still on screen. If the user cleared the screen with a ctrl-L, this won't be true. TODO -- can we get a callback at the clear and just delete the mark there? */
    if (TRUE == gtk_text_iter_is_end(&insertion_point)) {
        purple_debug_warning(PLUGIN_ID, "Not adding the icon, since the insertion point is no longer in the buffer\n");
    } else {
        /* insert icon actually */
        gtk_text_buffer_insert_pixbuf(target_buffer, &insertion_point, conv_icon->pixbuf);
    }

    gtk_text_buffer_delete_mark(target_buffer, requested_mark);
    requested_mark = NULL;
    purple_debug_info(PLUGIN_ID, "inserted icon into conv\n");
}

static void insert_requested_icon(TwitterConvIcon * conv_icon)
{
    GList          *mark_list = NULL;

    if (!conv_icon)
        return;

    mark_list = conv_icon->request_list;

    purple_debug_info(PLUGIN_ID, "about to insert icon for pending requests\n");

    if (mark_list) {
        g_list_foreach(mark_list, (GFunc) insert_icon_at_mark, conv_icon);
        mark_list = g_list_remove_all(mark_list, NULL);
        g_list_free(mark_list);
        conv_icon->request_list = NULL;
    }

}

static void got_page_cb(PurpleUtilFetchUrlData * url_data, gpointer user_data, const gchar * url_text, gsize len, const gchar * error_message)
{
    BuddyIconContext *ctx = user_data;
    TwitterConvIcon *conv_icon;
    const gchar    *pic_data;

    conv_icon = twitter_conv_icon_find(ctx->account, ctx->buddy_name);
    twitter_buddy_icon_context_free(ctx);

    g_return_if_fail(conv_icon != NULL);

    conv_icon->requested = FALSE;
    conv_icon->fetch_data = NULL;

    if (len && !error_message && twitter_response_text_status_code(url_text) == 200 && (pic_data = twitter_response_text_data(url_text, len))) {
        purple_debug_info(PLUGIN_ID, "Attempting to create pixbuf\n");
        purple_debug_misc(PLUGIN_ID, "MHM: url text is |%s|\n", url_text);
        purple_debug_misc(PLUGIN_ID, "MHM: url text is |%02x %02x %02x %02x|\n", pic_data[0], pic_data[1], pic_data[2], pic_data[3]);
        conv_icon->pixbuf = make_scaled_pixbuf((const guchar *) pic_data, len);
    }

    if (conv_icon->pixbuf) {
        purple_debug_info(PLUGIN_ID, "All succeeded, inserting\n");
        insert_requested_icon(conv_icon);
    }
}

void twitter_conv_icon_got_user_icon(PurpleAccount * account, const char *user_name, const gchar * url, time_t icon_time)
{
    /* look local icon cache for the requested icon */
    PurpleConnection *gc = purple_account_get_connection(account);
    TwitterConnectionData *twitter = gc->proto_data;
    TwitterConvIcon *conv_icon = NULL;
    GHashTable     *hash = twitter->icons;

    if (!hash)
        return;

    /* since this function is called after mark_icon_for_user(), data
     * must exist here. */
    conv_icon = twitter_conv_icon_find(account, user_name);
    if (!conv_icon) {
        conv_icon = twitter_conv_icon_new(account, user_name);
        g_hash_table_insert(hash, g_strdup(purple_normalize(account, user_name)), conv_icon);
        conv_icon->mtime = icon_time;
    } else {
        //A new icon is one posted with a tweet later than the current saved icon time
        //and with a different url
        gboolean        new_icon = !conv_icon->icon_url || (strcmp(url, conv_icon->icon_url) && icon_time > conv_icon->mtime);

        purple_debug_info(PLUGIN_ID, "Have icon %s (%lld) for user %s, looking for %s (%lld)\n", conv_icon->icon_url, (long long int) conv_icon->mtime, user_name, url, (long long int) icon_time);

        if (icon_time > conv_icon->mtime)
            conv_icon->mtime = icon_time;

        //Return if the image is cached already and it's the same one
        if (conv_icon->pixbuf && !new_icon)
            return;

        /* Return if user's icon has been requested already. */
        if (conv_icon->requested && !new_icon)
            return;

        //If we're already requesting, but it's a different url, cancel the fetch
        if (conv_icon->fetch_data)
            purple_util_fetch_url_cancel(conv_icon->fetch_data);

        conv_icon_clear(conv_icon);
    }

    conv_icon->icon_url = g_strdup(url);

    //For buddies, we don't want to retrieve the icon here, we'll
    //let the twitter_buddy fetch the icon and let us know when it's done
    if (purple_find_buddy(account, user_name))
        return;

    conv_icon->requested = TRUE;

    /* Create the URL for an user's icon. */
    if (url) {
        BuddyIconContext *ctx = twitter_buddy_icon_context_new(account, user_name, url);
        purple_debug_info(PLUGIN_ID, "requesting %s for %s\n", url, user_name);
        conv_icon->fetch_data = purple_util_fetch_url_request_len_with_account(account, url, TRUE, NULL, FALSE, NULL, TRUE, -1, got_page_cb, ctx);
    }
}

static void twitter_conv_icon_free(TwitterConvIcon * conv_icon)
{
    if (!conv_icon)
        return;
    purple_debug_info(PLUGIN_ID, "Freeing icon for %s\n", conv_icon->username);
    if (conv_icon->requested) {
        if (conv_icon->fetch_data) {
            purple_util_fetch_url_cancel(conv_icon->fetch_data);
        }
        conv_icon->fetch_data = NULL;
        conv_icon->requested = FALSE;
    }
    if (conv_icon->request_list)
        remove_marks_func(conv_icon, NULL);
    conv_icon->request_list = NULL;

    if (conv_icon->pixbuf) {
        g_object_unref(G_OBJECT(conv_icon->pixbuf));
    }
    conv_icon->pixbuf = NULL;

    if (conv_icon->icon_url)
        g_free(conv_icon->icon_url);
    conv_icon->icon_url = NULL;

    g_free(conv_icon->username);
    conv_icon->username = NULL;

    g_free(conv_icon);
}

static void mark_icon_for_user(GtkTextMark * mark, TwitterConvIcon * conv_icon)
{
    conv_icon->request_list = g_list_prepend(conv_icon->request_list, mark);
}

static gboolean twitter_conv_icon_displaying_chat_cb(PurpleAccount * account, const char *who, char **message, PurpleConversation * conv, PurpleMessageFlags flags, void *account_signal)
{
    GtkIMHtml      *imhtml;
    GtkTextBuffer  *text_buffer;
    gint            linenumber = 0;

    if (account != account_signal)
        return FALSE;

    purple_debug_info(PLUGIN_ID, "called %s\n", G_STRFUNC);

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
    purple_conversation_set_data(conv, PLUGIN_ID "-icon-ln", GINT_TO_POINTER(linenumber));

    return FALSE;
}

static void twitter_conv_icon_add_conv(TwitterConvIcon * conv_icon, PurpleConversation * conv)
{
    GList         **conv_conv_icons = purple_conversation_get_data(conv, PLUGIN_ID "-conv-icons");
    g_return_if_fail(conv_conv_icons != NULL);

    if (!g_list_find(conv_icon->convs, conv)) {
        conv_icon->convs = g_list_prepend(conv_icon->convs, conv);
        *conv_conv_icons = g_list_prepend(*conv_conv_icons, conv_icon);
    }
}

static void twitter_conv_icon_remove_conv(TwitterConvIcon * conv_icon, PurpleConversation * conv)
{
    conv_icon->convs = g_list_remove(conv_icon->convs, conv);

    if (!conv_icon->convs) {
        PurpleAccount  *account = purple_conversation_get_account(conv);
        PurpleConnection *gc = purple_account_get_connection(account);
        TwitterConnectionData *twitter = gc->proto_data;
        //Free the conv icon
        g_hash_table_remove(twitter->icons, conv_icon->username);
    }
}

static void twitter_conv_icon_displayed_chat_cb(PurpleAccount * account, const char *who, char *message, PurpleConversation * conv, PurpleMessageFlags flags, void *account_signal)
{
    GtkIMHtml      *imhtml;
    GtkTextBuffer  *text_buffer;
    GtkTextIter     insertion_point;
    gint            linenumber;
    TwitterConvIcon *conv_icon;
    PurpleConnection *gc;
    TwitterConnectionData *twitter;

    if (account != account_signal)
        return;

    gc = purple_account_get_connection(account);
    if (!gc) {
        return;
    }

    twitter = gc->proto_data;

    purple_debug_info(PLUGIN_ID, "%s\n", G_STRFUNC);

    /* insert icon */
    imhtml = GTK_IMHTML(PIDGIN_CONVERSATION(conv)->imhtml);
    text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(imhtml));

    /* get GtkTextIter in the target line */
    linenumber = GPOINTER_TO_INT(purple_conversation_get_data(conv, PLUGIN_ID "-icon-ln"));
    gtk_text_buffer_get_iter_at_line(text_buffer, &insertion_point, linenumber);

    conv_icon = twitter_conv_icon_find(account, who);

    /* if we don't have the icon for this user, put a mark instead and
     * request the icon */
    if (!conv_icon) {
        conv_icon = twitter_conv_icon_new(account, who);
        twitter_conv_icon_add_conv(conv_icon, conv);
        g_hash_table_insert(twitter->icons, g_strdup(purple_normalize(account, who)), conv_icon);
        mark_icon_for_user(gtk_text_buffer_create_mark(text_buffer, NULL, &insertion_point, FALSE), conv_icon);
        return;
    }
    twitter_conv_icon_add_conv(conv_icon, conv);

    /* if we have the icon for this user, insert the icon immediately */
    if (TRUE) {
        if (conv_icon->pixbuf) {
            gtk_text_buffer_insert_pixbuf(text_buffer, &insertion_point, conv_icon->pixbuf);
        } else {
            mark_icon_for_user(gtk_text_buffer_create_mark(text_buffer, NULL, &insertion_point, FALSE), conv_icon);
        }
    }

    purple_debug_info(PLUGIN_ID, "end %s\n", G_STRFUNC);
}

static void twitter_conv_icon_remove_conversation_conv_icons(PurpleConversation * conv)
{
    GList         **conv_icons;
    GList          *l;
    g_return_if_fail(conv != NULL);
    purple_debug_info(PLUGIN_ID, "%s conv %s\n", G_STRFUNC, purple_conversation_get_name(conv));

    conv_icons = purple_conversation_get_data(conv, PLUGIN_ID "-conv-icons");

    if (conv_icons) {
        for (l = *conv_icons; l; l = l->next) {
            TwitterConvIcon *conv_icon = l->data;
            twitter_conv_icon_remove_conv(conv_icon, conv);
        }
        g_list_free(*conv_icons);
        *conv_icons = NULL;
    }
}

static void twitter_conv_icon_deleting_conversation_cb(PurpleConversation * conv, PurpleAccount * account)
{
    GList         **conv_icons;
    if (purple_conversation_get_account(conv) != account)
        return;

    twitter_conv_icon_remove_conversation_conv_icons(conv);

    conv_icons = purple_conversation_get_data(conv, PLUGIN_ID "-conv-icons");
    g_free(conv_icons);
}

static void twitter_conv_icon_conversation_created_cb(PurpleConversation * conv, PurpleAccount * account)
{
    GList         **conv_icons;
    if (purple_conversation_get_account(conv) != account)
        return;
    conv_icons = g_new0(GList *, 1);

    purple_conversation_set_data(conv, PLUGIN_ID "-conv-icons", conv_icons);
}

void twitter_conv_icon_account_load(PurpleAccount * account)
{
    PurpleConnection *gc = purple_account_get_connection(account);
    TwitterConnectionData *twitter = gc->proto_data;
    twitter->icons = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify) twitter_conv_icon_free);

    purple_signal_connect(pidgin_conversations_get_handle(), "displaying-chat-msg", twitter->icons, PURPLE_CALLBACK(twitter_conv_icon_displaying_chat_cb), account);
    purple_signal_connect(pidgin_conversations_get_handle(), "displayed-chat-msg", twitter->icons, PURPLE_CALLBACK(twitter_conv_icon_displayed_chat_cb), account);
    purple_signal_connect(purple_conversations_get_handle(), "conversation-created", twitter->icons, PURPLE_CALLBACK(twitter_conv_icon_conversation_created_cb), account);
    purple_signal_connect(purple_conversations_get_handle(), "deleting-conversation", twitter->icons, PURPLE_CALLBACK(twitter_conv_icon_deleting_conversation_cb), account);
}

void twitter_conv_icon_account_unload(PurpleAccount * account)
{
    PurpleConnection *gc = purple_account_get_connection(account);
    TwitterConnectionData *twitter;
    GList          *l;

    /* Remove icons from all conversations */
    for (l = purple_get_chats(); l; l = l->next) {
        if (purple_conversation_get_account(l->data) == account)
            twitter_conv_icon_remove_conversation_conv_icons(l->data);
    }
    if (gc && (twitter = gc->proto_data)) {
        if (twitter->icons) {
            purple_signals_disconnect_by_handle(twitter->icons);
            g_hash_table_destroy(twitter->icons);
        }
        twitter->icons = NULL;
    }
}
