#ifndef _TWITTER_PLUGIN_H_
#define _TWITTER_PLUGIN_H_

typedef struct _TwitterRequestor TwitterRequestor;

typedef struct {
	const gchar *verify_credentials;
} TwitterUrls;

void            twitter_destroy(PurplePlugin * plugin);
void            twitter_tooltip_text(PurpleBuddy * buddy, PurpleNotifyUserInfo * info, gboolean full);
GList          *twitter_actions(PurplePlugin * plugin, gpointer context);
GHashTable     *prpltwtr_get_account_text_table_statusnet(PurpleAccount * account);
const char     *twitter_list_icon(PurpleAccount * account, PurpleBuddy * buddy);
char           *twitter_status_text(PurpleBuddy * buddy);
GList          *twitter_status_types(PurpleAccount * account);
GList          *twitter_blist_node_menu(PurpleBlistNode * node);
GList          *twitter_chat_info(PurpleConnection * gc);
GHashTable     *twitter_chat_info_defaults(PurpleConnection * gc, const char *chat_name);
void            twitter_close(PurpleConnection * gc);
int             twitter_send_im(PurpleConnection * gc, const char *conv_name, const char *message, PurpleMessageFlags flags);
void            twitter_set_info(PurpleConnection * gc, const char *info);
void            twitter_set_status(PurpleAccount * account, PurpleStatus * status);
void            twitter_add_buddy(PurpleConnection * gc, PurpleBuddy * buddy, PurpleGroup * group);
void            twitter_add_buddies(PurpleConnection * gc, GList * buddies, GList * groups);
void            twitter_remove_buddy(PurpleConnection * gc, PurpleBuddy * buddy, PurpleGroup * group);
void            twitter_remove_buddies(PurpleConnection * gc, GList * buddies, GList * groups);
void            twitter_chat_join(PurpleConnection * gc, GHashTable * components);
void            twitter_chat_leave(PurpleConnection * gc, int id);
int             twitter_chat_send(PurpleConnection * gc, int id, const char *message, PurpleMessageFlags flags);
void            twitter_get_cb_info(PurpleConnection * gc, int id, const char *who);
char           *twitter_chat_get_name(GHashTable * components);
void            twitter_convo_closed(PurpleConnection * gc, const gchar * conv_name);
void            twitter_set_buddy_icon(PurpleConnection * gc, PurpleStoredImage * img);
void            prpltwtr_plugin_init(PurplePlugin * plugin);

/// Configures the requestor with the specific paths and formats used by this account.
/// This will pre-calculate all the known paths used by the plugin (excluding those with
/// additional parameters). In addition, it will set up the pointers to parse the data
/// from the social network.
///
/// This will pick the appropriate network based on the account in the requestor.
void            prpltwtr_plugin_setup(TwitterRequestor * requestor);

#endif
