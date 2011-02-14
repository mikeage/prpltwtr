#
# gtkprpltwtr protocol plug-in
#

all: build 

TOPLEVEL := ../

include ../global.mak

TARGETS = gtkprpltwtr$(PLUGIN_SUFFIX)

LD = $(CC)

ifeq ($(strip $(IS_WIN32)), 1)

GTKPRPLTWTR_INC_PATHS += -I$(GTK_TOP)/include/gtk-2.0 \
			-I$(GTK_TOP)/include/pango-1.0 \
			-I$(GTK_TOP)/include/atk-1.0 \
			-I$(GTK_TOP)/include/cairo \
			-I$(GTK_TOP)/lib/gtk-2.0/include \
			-I$(PIDGIN_TOP)/win32 \
			-I$(PIDGIN_TOP)


LIB_PATHS += -L$(GTK_TOP)/lib \
			-L$(PURPLE_TOP) \
			-L$(PIDGIN_TOP)
			
LIBS =	-lgtk-win32-2.0 \
			-lglib-2.0 \
			-lgdk_pixbuf-2.0 \
			-lgdk-win32-2.0 \
			-lgobject-2.0 \
			-lintl \
			-lpurple \
			-lpidgin
CFLAGS := $(PURPLE_CFLAGS) $(GTKPRPLTWTR_INC_PATHS)
else
IS_PIDGIN := 1
PIDGIN_NAME := pidgin
PIDGIN_LIBS = $(shell pkg-config --libs $(PIDGIN_NAME))
PIDGIN_CFLAGS = $(CFLAGS) -DPIDGIN_PLUGINS -DENABLE_NLS -DPRPL_TWITTER_VERSION=\"$(VERSION)\"
PIDGIN_CFLAGS += $(shell pkg-config --cflags $(PIDGIN_NAME))
CFLAGS := $(PURPLE_CFLAGS) $(PIDGIN_CFLAGS)
LIB_PATHS = 
LIBS = $(PIDGIN_LIBS)
endif

GTKPRPLTWTR_C_SRC = gtkprpltwtr.c twitter_convicon.c twitter_charcount.c gtkprpltwtr_prefs.c $(TWITTER_LIB_SRC:%=../%)
GTKPRPLTWTR_H_SRC = $(GTKPRPLTWTR_C_SRC:%.c=%.h)
GTKPRPLTWTR_OBJ = $(GTKPRPLTWTR_C_SRC:%.c=%.o)

DISTFILES = gtkprpltwtr.c Makefile

OBJECTS = $(GTKPRPLTWTR_OBJ)

.PHONY: clean install build

build: $(TARGETS)

install: $(TARGETS)
	rm -f $(PURPLE_PLUGIN_DIR)/gtkprpltwtr$(PLUGIN_SUFFIX)
	install -m 0755 -d $(PURPLE_PLUGIN_DIR)
	cp gtkprpltwtr$(PLUGIN_SUFFIX) $(PURPLE_PLUGIN_DIR)/gtkprpltwtr$(PLUGIN_SUFFIX)

uninstall: 
	rm -f $(PURPLE_PLUGIN_DIR)/gtkprpltwtr$(PLUGIN_SUFFIX)

clean:
	rm -f $(TARGETS) $(OBJECTS) *.dbgsym

gtkprpltwtr$(PLUGIN_SUFFIX): $(GTKPRPLTWTR_OBJ)
	$(LD) $(LDFLAGS) -shared $(GTKPRPLTWTR_OBJ) $(LIB_PATHS) $(LIBS) $(DLL_LD_FLAGS) -o gtkprpltwtr$(PLUGIN_SUFFIX).dbgsym
	$(STRIP) -g gtkprpltwtr$(PLUGIN_SUFFIX).dbgsym -o gtkprpltwtr$(PLUGIN_SUFFIX)


