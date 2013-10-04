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

#include <glib.h>
#include <debug.h>
#include <request.h>

#include "prpltwtr_format.h"
#include "prpltwtr_format_xml.h"
#include "prpltwtr_xml.h"

void prpltwtr_format_xml_free_node(gpointer node)
{
	xmlnode_free(node);
}

gpointer prpltwtr_format_xml_copy_node(gpointer node)
{
	return xmlnode_copy(node);
}

gpointer prpltwtr_format_xml_from_str(const gchar * response, int response_length)
{
	purple_debug_info("prpltwtr", "DREM prpltwtr_format_xml_from_str\n");
	return xmlnode_from_str(response, response_length);
}

gpointer prpltwtr_format_xml_get_node(gpointer node, const gchar *child_name)
{
	return xmlnode_get_child_data(node, child_name);
}

gchar *prpltwtr_format_xml_get_str(gpointer node, const gchar *child_name)
{
	return xmlnode_get_child_data(node, child_name);
}

const gchar *prpltwtr_format_xml_node_parse_error(gpointer node)
{
	purple_debug_info("prpltwtr", "DREM prpltwtr_format_xml_node_parse_error\n");
	xmlnode *xml_node = node;
    return xmlnode_get_child_data(xml_node, "error");
}

void prpltwtr_format_xml_setup(TwitterFormat *format)
{
	format->extension = ".xml";

	format->copy_node = prpltwtr_format_xml_copy_node;
	format->free_node = prpltwtr_format_xml_free_node;
	format->from_str = prpltwtr_format_xml_from_str;
	format->get_node = prpltwtr_format_xml_get_node;
	format->get_str = prpltwtr_format_xml_get_str;
	format->parse_error = prpltwtr_format_xml_node_parse_error;
}
