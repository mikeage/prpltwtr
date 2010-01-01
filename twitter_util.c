/**
 * Copyright (C) 2009 Tan Miaoqing
 * Contact: Tan Miaoqing <rabbitrun84@gmail.com>
 */

#include <string.h>
#include "twitter_util.h"

gchar *xmlnode_get_child_data(const xmlnode *node, const char *name)
{
    xmlnode *child = xmlnode_get_child(node, name);
    if (!child)
        return NULL;
    return xmlnode_get_data_unescaped(child);
}
