#
# Global header for all makefile
#

-include $(TOPLEVEL)version.mak

# PIDGIN_TREE_TOP is only meaningful on Windows, point it to top directory of Pidgin. IT MUST BE A RELATIVE PATH
PIDGIN_TREE_TOP := $(TOPLEVEL)../pidgin-2.7.9

TWITTER_LIB_SRC = twitter_prefs.c twitter_request.c twitter_api.c twitter_search.c twitter_util.c twitter_xml.c twitter_endpoint_chat.c twitter_endpoint_search.c twitter_endpoint_timeline.c twitter_buddy.c twitter_conn.c twitter_endpoint_im.c twitter_endpoint_dm.c twitter_endpoint_reply.c twitter_mbprefs.c
TWITTER_C_SRC = twitter.c $(TWITTER_LIB_SRC)
TWITTER_OBJ = $(TWITTER_C_SRC:%.c=%.o)

# For Linux
DESTDIR := 

# Is this WIN32?
# This must be overwritten in local.mak for cross-compiling
IS_WIN32 = $(shell (uname -a | grep -q -i cygwin) && echo 1 || echo 0)

# for override those attributes
-include local.mak

ifeq ($(strip $(IS_WIN32)), 1)
# WIN32
# Use makefile and headers supplied by Pidgin 

PLUGIN_SUFFIX := .dll
EXE_SUFFIX := .exe

#include global makefile for WIN32 build
include $(PIDGIN_TREE_TOP)/libpurple/win32/global.mak

##
## INCLUDE PATHS
##
INCLUDE_PATHS +=	-I. \
			-I$(GTK_TOP)/include \
			-I$(GTK_TOP)/include/glib-2.0 \
			-I$(GTK_TOP)/lib/glib-2.0/include \
			-I$(PURPLE_TOP) \
			-I$(PURPLE_TOP)/win32 \
			-I$(PIDGIN_TREE_TOP)


LIBS += -lglib-2.0 \
			-lintl \
			-lws2_32 \
			-lpurple
			
PURPLE_LIBS = -L$(GTK_TOP)/lib -L$(PURPLE_TOP) $(LIBS)
PURPLE_CFLAGS = -DPURPLE_PLUGINS -DENABLE_NLS -Wdeclaration-after-statement -Wall -DPRPL_TWITTER_VERSION=\"$(VERSION)\" $(INCLUDE_PATHS) -g

PURPLE_PROTOCOL_PIXMAP_DIR = $(PURPLE_INSTALL_DIR)/pixmaps/pidgin/protocols
PURPLE_PLUGIN_DIR = $(PURPLE_INSTALL_PLUGINS_DIR)
LDFLAGS := -g
else #LINUX

LBITS := $(shell getconf LONG_BIT)

PREFIX := $(shell pkg-config --variable=prefix purple 2> /dev/null || echo /usr)
ifeq ($(LBITS),64)
LIBDIR := $(PREFIX)/lib64
else
LIBDIR := $(PREFIX)/lib
endif

# LINUX and others, use pkg-config
PURPLE_LIBS = $(shell pkg-config --libs purple)
PURPLE_CFLAGS = $(CFLAGS) -DPURPLE_PLUGINS -DENABLE_NLS -DPRPL_TWITTER_VERSION=\"$(VERSION)\" 
PURPLE_CFLAGS += $(shell pkg-config --cflags purple)
PURPLE_CFLAGS += -Wdeclaration-after-statement -Wall -pthread -I. -g -O2 -pipe -fPIC -DPIC 
PLUGIN_SUFFIX := .so
EXE_SUFFIX := 
STRIP := strip

PURPLE_PROTOCOL_PIXMAP_DIR := $(DESTDIR)$(PREFIX)/share/pixmaps/pidgin/protocols
PURPLE_PLUGIN_DIR := $(DESTDIR)$(LIBDIR)/purple-2

LDFLAGS := $(shell (echo $(PURPLE_CFLAGS)| tr ' ' '\n' | awk '!a[$$0]++' | tr '\n' ' '))
endif
