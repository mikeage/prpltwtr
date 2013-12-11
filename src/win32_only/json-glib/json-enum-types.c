
/* Generated data (by glib-mkenums) */

#include "json-enum-types.h"
/* enumerations from "../json-glib/json-parser.h" */
#include "../json-glib/json-parser.h"
GType
json_parser_error_get_type(void) {
  static GType etype = 0;
  if (G_UNLIKELY (!etype))
    {
      const GEnumValue values[] = {
        { JSON_PARSER_ERROR_PARSE, "JSON_PARSER_ERROR_PARSE", "parse" },
        { JSON_PARSER_ERROR_UNKNOWN, "JSON_PARSER_ERROR_UNKNOWN", "unknown" },
        { 0, NULL, NULL }
      };
      etype = g_enum_register_static (g_intern_static_string ("JsonParserError"), values);
    }
  return etype;
}

/* enumerations from "../json-glib/json-types.h" */
#include "../json-glib/json-types.h"
GType
json_node_type_get_type(void) {
  static GType etype = 0;
  if (G_UNLIKELY (!etype))
    {
      const GEnumValue values[] = {
        { JSON_NODE_OBJECT, "JSON_NODE_OBJECT", "object" },
        { JSON_NODE_ARRAY, "JSON_NODE_ARRAY", "array" },
        { JSON_NODE_VALUE, "JSON_NODE_VALUE", "value" },
        { JSON_NODE_NULL, "JSON_NODE_NULL", "null" },
        { 0, NULL, NULL }
      };
      etype = g_enum_register_static (g_intern_static_string ("JsonNodeType"), values);
    }
  return etype;
}


/* Generated data ends here */

