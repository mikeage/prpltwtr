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

#include "defaults.h"
#include "prpltwtr_format.h"
#include "prpltwtr_format_json.h"

typedef struct {
    JsonArray      *array;
    gpointer        node;
    gint            index;
    gint            count;
} _TwitterJsonIter;

static gchar   *json_get_str(gpointer node, const gchar * child_node_name);
static void     json_free_node(gpointer node);
static GList   *json_copy_into(gpointer node, GList * list, gint * count_ref);
static gpointer json_copy_node(gpointer node);
static gpointer json_from_str(const gchar * response, int response_length);
static gchar   *json_get_attr(gpointer node, const gchar * attr_name);
static gpointer json_get_iter_node(gpointer iter);
static gchar   *json_get_name(gpointer node);
static gpointer json_get_node(gpointer node, const gchar * child_node_name);
static gint     json_get_node_child_count(gpointer node);
static gchar   *json_get_str(gpointer node, const gchar * child_node_name);
static gboolean json_is_name(gpointer node, const gchar * child_name);
static gpointer json_iter_start(gpointer node, const gchar * child_name);
static gboolean json_iter_done(gpointer iter);
static gpointer json_iter_next(gpointer iter);
static const gchar *json_node_parse_error(gpointer node);
;
static void json_free_node(gpointer node)
{
    json_node_free(node);
}

static GList   *json_copy_into(gpointer node, GList * list, gint * count_ref)
{
    JsonArray      *array = NULL;
    int             count;
    int             i;
    purple_debug_info(GENERIC_PROTOCOL_ID, "BEGIN: %s: is array %d\n", G_STRFUNC, JSON_NODE_TYPE(node) == JSON_NODE_ARRAY);

    if (JSON_NODE_TYPE(node) != JSON_NODE_ARRAY) {
        purple_debug_info(GENERIC_PROTOCOL_ID, "END: %s: incorrect data type\n", G_STRFUNC);
        return list;
    }

    array = json_node_get_array(node);
    count = json_array_get_length(array);

    purple_debug_info(GENERIC_PROTOCOL_ID, "MIDDLE: %s: count %d\n", G_STRFUNC, count);

    for (i = 0; i < count; i++) {
        JsonNode       *child = json_array_get_element(array, i);
        JsonNode       *copy = json_node_copy(child);
        list = g_list_prepend(list, copy);
    }

    purple_debug_info(GENERIC_PROTOCOL_ID, "END: %s\n", G_STRFUNC);

    *count_ref = count;

    return list;
}

static gpointer json_copy_node(gpointer node)
{
    JsonNode       *copy = json_node_copy(node);

    purple_debug_info(GENERIC_PROTOCOL_ID, "BEGIN: %s: node %d\n", G_STRFUNC, JSON_NODE_TYPE(node));

    return copy;
}

static gpointer json_from_str(const gchar * response, int response_length)
{
    JsonParser     *parser = json_parser_new();
    JsonNode       *root;

    GError         *error = NULL;
    //purple_debug_info(GENERIC_PROTOCOL_ID, "%s: %s", G_STRFUNC, response);
    json_parser_load_from_data(parser, response, response_length, &error);

    if (error) {
        g_print("Unable to parse `%s': %s\n", response, error->message);
        g_error_free(error);
        g_object_unref(parser);
        return NULL;
    }

    root = json_parser_get_root(parser);
    purple_debug_info(GENERIC_PROTOCOL_ID, "%s: isObject %d isArray %d\n", G_STRFUNC, JSON_NODE_TYPE(root) == JSON_NODE_OBJECT, JSON_NODE_TYPE(root) == JSON_NODE_ARRAY);
    return root;
}

static gchar   *json_get_attr(gpointer node, const gchar * attr_name)
{
    if (JSON_NODE_TYPE(node) != JSON_NODE_OBJECT)
        return NULL;

    return json_get_str(node, attr_name);
}

static gpointer json_get_iter_node(gpointer iter)
{
    _TwitterJsonIter *json_iter = iter;
    return json_iter->node;
}

static gchar   *json_get_name(gpointer node)
{
    const gchar    *name = json_node_type_name(node);
    return g_strdup(name);
}

static gpointer json_get_node(gpointer node, const gchar * child_node_name)
{
    JsonObject     *node_object;
    JsonNode       *child;
    if (JSON_NODE_TYPE(node) != JSON_NODE_OBJECT)
        return NULL;

    node_object = json_node_get_object(node);

    // If we don't have the member, then return a NULL which indicates no error.
    if (!json_object_has_member(node_object, child_node_name)) {
        return NULL;
    }

    child = json_object_get_member(node_object, child_node_name);
    return child;
}

static gint json_get_node_child_count(gpointer node)
{
    purple_debug_info(GENERIC_PROTOCOL_ID, "BEGIN: %s\n", G_STRFUNC);

    if (JSON_NODE_TYPE(node) == JSON_NODE_OBJECT) {
        JsonObject     *node_obj = json_node_get_object(node);
        int             count = json_object_get_size(node_obj);
        purple_debug_info(GENERIC_PROTOCOL_ID, "END: %s: object %d\n", G_STRFUNC, count);
        return count;

    } else {
        JsonArray      *node_array = json_node_get_array(node);
        int             count = json_array_get_length(node_array);
        purple_debug_info(GENERIC_PROTOCOL_ID, "END: %s: array %d\n", G_STRFUNC, count);
        return count;
    }
}

static gchar   *json_get_str(gpointer node, const gchar * child_node_name)
{
    JsonObject     *node_object;
    const gchar    *const_value;
    gchar          *child_value;

    if (JSON_NODE_TYPE(node) != JSON_NODE_OBJECT)
        return NULL;

    node_object = json_node_get_object(node);

    // If we don't have the member, then return a NULL which indicates no error.
    if (!json_object_has_member(node_object, child_node_name)) {
        return NULL;
    }

    const_value = json_object_get_string_member(node_object, child_node_name);

    if (!g_strcmp0(const_value, "(null)")) {
        return NULL;
    }

    child_value = g_strdup(const_value);
    return child_value;
}

static gboolean json_is_name(gpointer node, const gchar * child_name)
{
    return TRUE;
}

static gpointer json_iter_start(gpointer node, const gchar * child_name)
{
    // Initialize the
    _TwitterJsonIter *iter = g_new0(_TwitterJsonIter, 1);
    iter->index = 0;

    // If we are currently in an array, then we just use it.
    if (JSON_NODE_TYPE(node) == JSON_NODE_ARRAY) {
        iter->array = json_node_get_array(node);
    } else {
        if (child_name == NULL) {
            purple_debug_info(GENERIC_PROTOCOL_ID, "ERROR: %s: Node is not an array and name is not provided\n", G_STRFUNC);
            return NULL;
        }
        iter->array = json_node_get_array(json_get_node(node, child_name));
    }

    // Populate the items.
    iter->count = json_array_get_length(iter->array);

    if (iter->count) {
        iter->node = json_array_get_element(iter->array, 0);
    } else {
        iter->node = NULL;
    }

    // Return the resulting iterator.
    return iter;
}

static gboolean json_iter_done(gpointer iter)
{
    return iter == NULL;
}

static gpointer json_iter_next(gpointer iter)
{
    _TwitterJsonIter *json_iter = iter;
    json_iter->index++;

    if (json_iter->index >= json_iter->count) {
        // Free the item since we've finished with it.
        g_free(json_iter);
        return NULL;
    }

    json_iter->node = json_array_get_element(json_iter->array, json_iter->index);
    return iter;
}

static const gchar *json_node_parse_error(gpointer node)
{
    return json_get_str(node, "error");
}

void prpltwtr_format_json_setup(TwitterFormat * format)
{
    format->extension = ".json";

    format->copy_into = json_copy_into;
    format->copy_node = json_copy_node;
    format->free_node = json_free_node;
    format->from_str = json_from_str;
    format->get_attr = json_get_attr;
    format->get_iter_node = json_get_iter_node;
    format->get_name = json_get_name;
    format->get_node = json_get_node;
    format->get_node_child_count = json_get_node_child_count;
    format->get_str = json_get_str;
    format->is_name = json_is_name;
    format->iter_start = json_iter_start;
    format->iter_done = json_iter_done;
    format->iter_next = json_iter_next;
    format->parse_error = json_node_parse_error;
}
