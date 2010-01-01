/**
 * Copyright (C) 2009 Tan Miaoqing
 * Contact: Tan Miaoqing <rabbitrun84@gmail.com>
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <glib.h>
#include <xmlnode.h>

gchar *xmlnode_get_child_data(const xmlnode *node, const char *name);

#endif /* UTIL_H_ */

#ifndef g_slice_new0
#define g_slice_new0(a) g_new0(a, 1)
#define g_slice_free(a, b) g_free(b)
#endif

#ifndef g_strcmp0
#define g_strcmp0(a, b) (a == NULL && b == NULL ? 0 : a == NULL ? -1 : b == NULL ? 1 : strcmp(a, b))
#endif
