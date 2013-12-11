#include "xmlnode_ext.h"

gchar          *xmlnode_get_child_data(const xmlnode * node, const char *name)
{
    xmlnode        *child = xmlnode_get_child(node, name);
    if (!child)
        return NULL;
    return xmlnode_get_data_unescaped(child);
}
