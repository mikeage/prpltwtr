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
typedef const gchar *(*TwitterFormatConstStringFromNodeFunc)(gpointer node);
typedef gchar *(*TwitterFormatStringFromNodeFunc)(gpointer node);
typedef gpointer (*TwitterFormatNodeFromNodeFunc)(gpointer node);
typedef gboolean (*TwitterFormatBoolFromNodeFunc)(gpointer node);
typedef gint (*TwitterFormatIntFromNodeFunc)(gpointer node);
typedef gchar *(*TwitterFormatStringFromChildNodeFunc)(gpointer node, const gchar *child_name);
typedef gpointer (*TwitterFormatNodeFromChildNodeFunc)(gpointer node, const gchar *child_name);

/// Contains function pointers for reading the output from the social network
/// and converting them into internal structures used by the plugin.
typedef struct {
	/// Contains the extension that is appended to the end of every request. This
	/// needs to include the "." if required (e.g., ".json" or ".xml").
	const gchar *extension;

	/// A function pointer that copies a given node and returns an identical,
	/// deep copy. This copy needs to be released.
	TwitterFormatNodeFromNodeFunc copy_node;
	
	/// A function pointer that releases the node returned by the from_str.
	/// This assumes the node is the one returned from from_str.
	TwitterFormatFromNodeFunc free_node;

	/// A function pointer for a method that takes a string and a length and
	/// produces an opaque pointer suitable for use by other methods in this
	/// structure.
	TwitterFormatNodeFromStringFunc from_str;

	/// A function pointer that retrieves a string from an attribute of the
	/// given node. The string parameter is the name of the attribute.
	TwitterFormatStringFromChildNodeFunc get_attr;

	/// A function pointer that retrieves the name of the given node and
	/// returns it.
	TwitterFormatStringFromNodeFunc get_name;
	
	/// A function pointer that gets a usable node from a given iterator. This
	/// node can be used with get_str and other functions.
	TwitterFormatNodeFromNodeFunc get_iter_node;

	/// A function pointer that retrieves a child node (also opaque) from the
	/// given node for the child_name element.
	TwitterFormatNodeFromChildNodeFunc get_node;
	
	/// A function pointer that returns the number of child elements
	/// underneath the given node.
	TwitterFormatIntFromNodeFunc get_node_child_count;
	
	/// A function pointer that retrieves a string from a child node of the
	/// given node.
	TwitterFormatStringFromChildNodeFunc get_str;

	/// A function pointer that takes the iterator from `iter_next` or
	/// `iter_start` and determines if a node (using `get_iter_node`) can be
	/// retrieved from it.
	TwitterFormatBoolFromNodeFunc iter_done;

	/// A function pointer that advances an iterator (from `iter_start`) to
	/// the next node.
	TwitterFormatNodeFromNodeFunc iter_next;

	/// A function pointer that gets an iterator from a child element of the
	/// supplied node. The name is the name of the child node.
	TwitterFormatNodeFromChildNodeFunc iter_start;

	/// A function pointer for a method that takes the opaque node and returns
	/// the error text inside it.
	TwitterFormatConstStringFromNodeFunc parse_error;
} TwitterFormat;

#endif
