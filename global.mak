#
# Global header for all makefile
#

-include version.mak

# PIDGIN_TREE_TOP is only meaningful on Windows, point it to top directory of Pidgin. IT MUST BE A RELATIVE PATH
PIDGIN_TREE_TOP := ../../pidgin-2.5.4


# For Linux
DESTDIR := 
PREFIX := $(shell pkg-config --variable=prefix $(PIDGIN_NAME) 2> /dev/null || echo /usr)
LIBDIR := $(PREFIX)/lib

# Is this WIN32?
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
			-I$(PIDGIN_TREE_TOP) \
			-I$(PIDGIN_TOP) \
			-I$(GTK_TOP)/include/gtk-2.0 \
			-I$(GTK_TOP)/include/pango-1.0 \
			-I$(GTK_TOP)/include/atk-1.0 \
			-I$(GTK_TOP)/include/cairo \
			-I$(GTK_TOP)/lib/gtk-2.0/include \
			-I$(PIDGIN_TOP)/win32  \


LIBS += -lglib-2.0 \
			-lintl \
			-lws2_32 \
			-lpurple \
			$(PIDGIN_TOP)/pidgin$(PLUGIN_SUFFIX)
			
PURPLE_LIBS = -L$(GTK_TOP)/lib -L$(PURPLE_TOP) $(LIBS)
PURPLE_CFLAGS = -DPURPLE_PLUGINS -DENABLE_NLS -Wall -DMBPURPLE_VERSION=\"$(VERSION)\" $(INCLUDE_PATHS)

PURPLE_PROTOCOL_PIXMAP_DIR = $(PURPLE_INSTALL_DIR)/pixmaps/pidgin/protocols
PURPLE_PLUGIN_DIR = $(PURPLE_INSTALL_PLUGINS_DIR)


#include $(PIDGIN_COMMON_RULES)

else

IS_PIDGIN = $(shell pkg-config --atleast-version=2.0 pidgin && echo 1 || echo 0)
IS_CARRIER = $(shell pkg-config --atleast-version=2.0 carrier && echo 1 || echo 0)

ifeq ($(strip $(IS_PIDGIN)), 1)
	PIDGIN_NAME := pidgin
else 
ifeq ($(strip $(IS_CARRIER)), 1)
        PIDGIN_NAME := carrier
endif
endif

PREFIX := $(shell pkg-config --variable=prefix $(PIDGIN_NAME) 2> /dev/null || echo /usr)
LIBDIR := $(PREFIX)/lib

# LINUX and others, use pkg-config
PURPLE_LIBS = $(shell pkg-config --libs purple)
PURPLE_CFLAGS = $(CFLAGS) -DPURPLE_PLUGINS -DENABLE_NLS -DMBPURPLE_VERSION=\"$(VERSION)\"
PURPLE_CFLAGS += $(shell pkg-config --cflags purple)
PURPLE_CFLAGS += $(shell pkg-config --cflags pidgin)
PURPLE_CFLAGS += -Wall -pthread -I. -g -O2 -pipe -fPIC -DPIC 
PLUGIN_SUFFIX := .so
EXE_SUFFIX := 

PURPLE_PROTOCOL_PIXMAP_DIR := $(DESTDIR)$(PREFIX)/share/pixmaps/pidgin/protocols
PURPLE_PLUGIN_DIR := $(DESTDIR)$(LIBDIR)/purple-2

PIDGIN_LIBS = $(shell pkg-config --libs $(PIDGIN_NAME))
PIDGIN_CFLAGS = $(CFLAGS) -DPIDGIN_PLUGINS -DENABLE_NLS -DMBPURPLE_VERSION=\"$(VERSION)\"
PIDGIN_CFLAGS += $(shell pkg-config --cflags $(PIDGIN_NAME))
PIDGIN_CFLAGS += -Wall -pthread -I. -g -O2 -pipe -fPIC -DPIC 

LDFLAGS := $(shell (echo $(PIDGIN_CFLAGS) $(PURPLE_CFLAGS)| tr ' ' '\n' | awk '!a[$$0]++' | tr '\n' ' '))
endif

dist: $(DISTFILES)
	mkdir -p ../$(PACKAGE)-$(VERSION)
	cp -f $(DISTFILES) ../$(PACKAGE)-$(VERSION)
