With larger systems, it can occasionally be overwhelming to see how the individual files work together. This is a general overview of how the code is organized.

# Plugin Files

* prpltwtr_plugin.h
* prpltwtr_plugin_twitter.c
* prpltwtr_plugin_statusnet.c

These contains the logic and structures for code specific to a given social network (e.g., Twitter, Status.Net). For each account type on the Pidgin list, there should be a single code file. The plugin files have two basic components: the libpurple code and the network abstraction logic.

TODO: prpltwtr_plugin.h should just include header files from the plugin files (e.g., prpltwtr_plugin_twitter.h) which would let each network be completely isolated (and controllable via #define and configuration options).

# Format Files

* prpltwtr_format.h
* prpltwtr_format_xml.h
* prpltwtr_format_xml.c
* prpltwtr_format_json.h
* prpltwtr_format_json.c

The format files encapsulate the logic for reading and writing the data from the network. These include not only convenience methods for handling the specific format (e.g., XML and JSON), but also more object-specific wrappers such as pulling out a tweet or status from the stream. These object-specific wrappers will typically not be used directly, but populated by the *Plugin* files and stored as function pointers in the `TwitterFormat` structure (which can be accessed by the `TwitterRequestor` (i.e., `requestor->format`).

The signatures for the function pointers are stored in `prpltwtr_format.h` which is usually the only file included. Only the plugin files usually need the format-specific headers.
