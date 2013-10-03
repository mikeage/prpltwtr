/**
 * TODO: legal stuff
 *
 * purple
 *
 * Purple is the legal property of its developers, whose names are too numerous
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

#ifndef _TWITTER_FORMAT_H_
#define _TWITTER_FORMAT_H_

#include <glib.h>

typedef gpointer (*TwitterFormatNodeFromStringFunc)(const gchar *response, int response_length);

typedef void (*TwitterFormatFromNodeFunc)(gpointer node);

typedef const gchar *(*TwitterFormatStringFromNodeFunc)(gpointer node);

/// Contains function pointers for reading the output from the social network
/// and converting them into internal structures used by the plugin.
typedef struct {
	/// Contains the extension that is appended to the end of every request. This
	/// needs to include the "." if required (e.g., ".json" or ".xml").
	const gchar *extension;

	/// A function pointer that releases the node returned by the from_str.
	TwitterFormatFromNodeFunc free_node;

	/// A function pointer for a method that takes a string and a length and
	/// produces an opaque pointer suitable for use by other methods in this
	/// structure.
	TwitterFormatNodeFromStringFunc from_str;

	/// A function pointer for a method that takes the opaque node and returns
	/// the error text inside it.
	TwitterFormatStringFromNodeFunc parse_error;
} TwitterFormat;

#endif
