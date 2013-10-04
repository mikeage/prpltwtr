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

#include <json-glib/json-glib.h>
#include <debug.h>

#include "prpltwtr_format.h"
#include "prpltwtr_format_json.h"

void prpltwtr_format_json_free_node(gpointer node)
{
	json_node_free(node);
}

gpointer prpltwtr_format_json_copy_node(gpointer node)
{
	JsonNode *copy = json_node_copy(node);
	return copy;
}

gpointer prpltwtr_format_json_from_str(const gchar * response, int response_length)
{
	JsonParser *parser = json_parser_new();
	
	GError *error = NULL;
	json_parser_load_from_data (parser, response, response_length, &error);
	
	if (error)
    {
		g_print ("Unable to parse `%s': %s\n", response, error->message);
		g_error_free (error);
		g_object_unref (parser);
		return NULL;
    }
	
	JsonNode *root = json_parser_get_root(parser);
	// DREM Need to release this at some point g_object_unref (parser);
	return root;
}

gpointer prpltwtr_format_json_get_node(gpointer node, const gchar *child_node_name)
{
	JsonObject *node_object = json_node_get_object(node);

	// If we don't have the member, then return a NULL which indicates no error.
	if (!json_object_has_member(node_object, child_node_name))
	{
		return NULL;
	}
	
	JsonNode *child = json_object_get_member(node_object, child_node_name);
	return child;
}

gchar *prpltwtr_format_json_get_str(gpointer node, const gchar *child_node_name)
{
	JsonObject *node_object = json_node_get_object(node);

	// If we don't have the member, then return a NULL which indicates no error.
	if (!json_object_has_member(node_object, child_node_name))
	{
		return NULL;
	}
	
	const gchar *const_value = json_object_get_string_member(node_object, child_node_name);
	gchar *child_value = g_strdup(const_value);
	return child_value;
}

const gchar *prpltwtr_format_json_node_parse_error(gpointer node)
{
	return prpltwtr_format_json_get_str(node, "error");
}

void prpltwtr_format_json_setup(TwitterFormat *format)
{
	format->extension = ".json";

	format->copy_node = prpltwtr_format_json_copy_node;
	format->free_node = prpltwtr_format_json_free_node;
	format->from_str = prpltwtr_format_json_from_str;
	format->get_node = prpltwtr_format_json_get_node;
	format->get_str = prpltwtr_format_json_get_str;
	format->parse_error = prpltwtr_format_json_node_parse_error;
}
