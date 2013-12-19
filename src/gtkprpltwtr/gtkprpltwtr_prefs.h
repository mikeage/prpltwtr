/*
 * prpltwtr 
 *
 * prpltwtr is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#ifndef _GTKPRPLTWTR_PREFS_H_
#define _GTKPRPLTWTR_PREFS_

#include "gtkprpltwtr.h"
#include <plugin.h>
#include <gtkplugin.h>

#define PREF_PREFIX "/plugins/gtk/" PLUGIN_ID

#define TWITTER_PREF_ENABLE_CONV_ICON PREF_PREFIX "/enable_conv_icon"
#define TWITTER_PREF_ENABLE_CONV_ICON_DEFAULT TRUE
#define TWITTER_PREF_CONV_ICON_SIZE PREF_PREFIX "/conv_icon_size"
#define TWITTER_PREF_CONV_ICON_SIZE_DEFAULT 32

void            gtkprpltwtr_prefs_init(PurplePlugin * plugin);
void            gtkprpltwtr_prefs_destroy(PurplePlugin * plugin);

#endif
