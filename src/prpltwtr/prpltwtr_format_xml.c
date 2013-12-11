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
    return xmlnode_from_str(response, response_length);
}

gchar          *prpltwtr_format_xml_get_attr(gpointer node, const gchar * attr_name)
{
    const gchar    *value = xmlnode_get_attrib(node, "rel");
    return g_strdup(value);
}

gpointer prpltwtr_format_xml_get_iter_node(gpointer iter)
{
    return iter;
}

gchar          *prpltwtr_format_xml_get_name(gpointer node)
{
    xmlnode        *xml = node;
    const gchar    *name = xml->name;
    return g_strdup(name);
}

gpointer prpltwtr_format_xml_get_node(gpointer node, const gchar * child_name)
{
    return xmlnode_get_child_data(node, child_name);
}

gint prpltwtr_format_xml_get_node_child_count(gpointer node)
{
    return xmlnode_child_count(node);
}

gchar          *prpltwtr_format_xml_get_str(gpointer node, const gchar * child_name)
{
    return xmlnode_get_child_data(node, child_name);
}

const gchar    *prpltwtr_format_xml_node_parse_error(gpointer node)
{
    xmlnode        *xml_node = node;
    return xmlnode_get_child_data(xml_node, "error");
}

gboolean prpltwtr_format_xml_is_name(gpointer node, const gchar * child_name)
{
    return prpltwtr_format_xml_get_name(node) && !strcmp(prpltwtr_format_xml_get_name(node), child_name);
}

gpointer prpltwtr_format_xml_iter_start(gpointer node, const gchar * child_name)
{
    return prpltwtr_format_xml_get_node(node, child_name);
}

gboolean prpltwtr_format_xml_iter_done(gpointer iter)
{
    return iter == NULL;
}

gpointer prpltwtr_format_xml_iter_next(gpointer iter)
{
    return xmlnode_get_next_twin(iter);
}

void prpltwtr_format_xml_setup(TwitterFormat * format)
{
    format->extension = ".xml";

    format->copy_node = prpltwtr_format_xml_copy_node;
    format->free_node = prpltwtr_format_xml_free_node;
    format->from_str = prpltwtr_format_xml_from_str;
    format->get_attr = prpltwtr_format_xml_get_attr;
    format->get_iter_node = prpltwtr_format_xml_get_iter_node;
    format->get_name = prpltwtr_format_xml_get_name;
    format->get_node = prpltwtr_format_xml_get_node;
    format->get_node_child_count = prpltwtr_format_xml_get_node_child_count;
    format->get_str = prpltwtr_format_xml_get_str;
    format->is_name = prpltwtr_format_xml_is_name;
    format->iter_start = prpltwtr_format_xml_iter_start;
    format->iter_done = prpltwtr_format_xml_iter_done;
    format->iter_next = prpltwtr_format_xml_iter_next;
    format->parse_error = prpltwtr_format_xml_node_parse_error;
}
