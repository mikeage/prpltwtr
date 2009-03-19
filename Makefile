#
# Microblog protocol plug-in
#

all: build 

include global.mak

TARGETS = libpurple-twitter-protocol$(PLUGIN_SUFFIX)

LD = $(CC)

ifeq ($(strip $(IS_WIN32)), 1)
TWITTER_INC_PATHS += -I$(GTK_TOP)/include/gtk-2.0 \
			-I$(GTK_TOP)/include/pango-1.0 \
			-I$(GTK_TOP)/include/atk-1.0 \
			-I$(GTK_TOP)/include/cairo \
			-I$(GTK_TOP)/lib/gtk-2.0/include \
			-I$(PIDGIN_TOP)/win32


LIB_PATHS += -L$(GTK_TOP)/lib \
			-L$(PURPLE_TOP) \
			-L$(PIDGIN_TOP)
			
LIBS =	-lgtk-win32-2.0 \
			-lglib-2.0 \
			-lgdk-win32-2.0 \
			-lgobject-2.0 \
			-lintl \
			-lpurple \
			-lpidgin
CFLAGS := $(PURPLE_CFLAGS) $(TWITTER_INC_PATHS)
else
CFLAGS := $(PURPLE_CFLAGS) $(PIDGIN_CFLAGS)
LIB_PATHS = 
LIBS = $(PIDGIN_LIBS)
endif

TWITTER_C_SRC = twitter.c twitter_request.c twitter_api.c
TWITTER_H_SRC = $(TWITTER_C_SRC:%.c=%.h) config.h
TWITTER_OBJ = $(TWITTER_C_SRC:%.c=%.o)

DISTFILES = $(TWITTER_C_SRC) $(TWITTER_H_SRC) Makefile global.mak version.mak

OBJECTS = $(TWITTER_OBJ)

.PHONY: clean install build

build: $(TARGETS)

install: $(TARGETS)
	rm -f $(PURPLE_PLUGIN_DIR)/libpurple-twitter-protocol$(PLUGIN_SUFFIX)
	install -m 0755 -d $(PURPLE_PLUGIN_DIR)
	cp libpurple-twitter-protocol$(PLUGIN_SUFFIX) $(PURPLE_PLUGIN_DIR)/libpurple-twitter-protocol$(PLUGIN_SUFFIX)

uninstall: 
	rm -f $(PURPLE_PLUGIN_DIR)/libpurple-twitter-protocol$(PLUGIN_SUFFIX)

clean:
	rm -f $(TARGETS) $(OBJECTS)

libpurple-twitter-protocol$(PLUGIN_SUFFIX): $(TWITTER_OBJ)
	$(LD) $(LDFLAGS) -shared $(TWITTER_OBJ) $(LIB_PATHS) $(LIBS) $(DLL_LD_FLAGS) -o libpurple-twitter-protocol$(PLUGIN_SUFFIX)

