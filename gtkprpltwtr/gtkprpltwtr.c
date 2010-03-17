#define PLUGIN_ID "gtkprpltwtr"

#include "../config.h"

#include <gtkplugin.h>
#include "../twitter.h"
#include "../twitter_charcount.h"


static PurplePlugin *gtkprpltwtr_plugin = NULL;

static gboolean plugin_load(PurplePlugin *plugin) 
{
	gtkprpltwtr_plugin = plugin;
	purple_signal_connect(purple_conversations_get_handle(),
			"conversation-created",
			plugin, PURPLE_CALLBACK(twitter_charcount_conv_created_cb), NULL);
	purple_signal_connect(purple_conversations_get_handle(),
			"deleting-conversation",
			plugin, PURPLE_CALLBACK(twitter_charcount_conv_destroyed_cb), NULL);
	return TRUE;
}

static gboolean plugin_unload(PurplePlugin *plugin)
{
	purple_signals_disconnect_by_handle(plugin);
	return TRUE;
}

static PurplePluginPrefFrame *get_plugin_pref_frame(PurplePlugin *plugin) 
{
	PurplePluginPrefFrame *frame;
	frame = purple_plugin_pref_frame_new();
	return frame;
}

void plugin_destroy(PurplePlugin * plugin)
{
}

static PurplePluginUiInfo prefs_info = {
	get_plugin_pref_frame,
	0,   /* page_num (Reserved) */
	NULL, /* frame (Reserved) */
	/* Padding */
	NULL,
	NULL,
	NULL,
	NULL
};

static PurplePluginInfo info =
{
	PURPLE_PLUGIN_MAGIC,
	PURPLE_MAJOR_VERSION,                           /**< major version */
	PURPLE_MINOR_VERSION,                           /**< minor version */
	PURPLE_PLUGIN_STANDARD,                         /**< type */
	PIDGIN_PLUGIN_TYPE,                             /**< ui_requirement */
	0,                                              /**< flags */
	NULL,                                           /**< dependencies */
	PURPLE_PRIORITY_DEFAULT,                        /**< priority */
	PLUGIN_ID,                                   /**< id */
	"GtkPrplTwtr",                                      /**< name */
	"0.5.0",                               /**< version */
	"PrplTwtr Pidgin Support",                        /**< summary */
	"PrplTwtr Pidgin Support",               /**< description */
	"neaveru <neaveru@gmail.com>",		     /* author */
	"http://code.google.com/p/prpltwtr/",  /* homepage */
	plugin_load,                                    /**< load */
	plugin_unload,                                  /**< unload */
	plugin_destroy,                                 /**< destroy */
	NULL,                                           /**< ui_info */
	NULL,                                           /**< extra_info */
	&prefs_info,                                    /**< prefs_info */
	NULL,                                           /**< actions */
	NULL,						    /* padding... */
	NULL,
	NULL,
	NULL,
};

static void plugin_init(PurplePlugin *plugin) 
{	
	gtkprpltwtr_plugin = plugin;
}

PURPLE_INIT_PLUGIN(gtkprpltwtr, plugin_init, info)
