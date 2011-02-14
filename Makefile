#
# Microblog protocol plug-in
#

all: build 

IS_PIDGIN = $(shell pkg-config --atleast-version=2.0 pidgin && echo 1 || echo 0)

include global.mak

ifeq ($(strip $(IS_WIN32)), 1)
IS_PIDGIN := 1
endif

SUBDIRS = data

ifeq ($(strip $(IS_PIDGIN)), 1)
SUBDIRS += gtkprpltwtr
endif

TARGETS = prpltwtr$(PLUGIN_SUFFIX)

LD = $(CC)

ifeq ($(strip $(IS_WIN32)), 1)
TWITTER_INC_PATHS += -I$(GTK_TOP)/include/gtk-2.0 \
			-I$(GTK_TOP)/include/pango-1.0 \
			-I$(GTK_TOP)/include/atk-1.0 \
			-I$(GTK_TOP)/include/cairo \
			-I$(GTK_TOP)/lib/gtk-2.0/include


LIB_PATHS += -L$(GTK_TOP)/lib \
			-L$(PURPLE_TOP)
			
LIBS =	-lglib-2.0 \
			-lintl \
			-lpurple
CFLAGS := $(PURPLE_CFLAGS) $(TWITTER_INC_PATHS)
else
CFLAGS := $(PURPLE_CFLAGS)
LIB_PATHS = 
endif

ifeq ($(PRPLTWTR_DEBUG), 1)
CFLAGS := $(CFLAGS) -D_DEBUG_=1
endif

TWITTER_H_SRC = $(TWITTER_C_SRC:%.c=%.h) config.h
TWITTER_OBJ = $(TWITTER_C_SRC:%.c=%.o)

DISTFILES = $(TWITTER_C_SRC) $(TWITTER_H_SRC) Makefile Makefile.telepathy global.mak version.mak twitter.nsi COPYING local.mak.mingw
DISTDIRS = $(SUBDIRS)

OBJECTS = $(TWITTER_OBJ)

.PHONY: clean install build

build: $(TARGETS)
	for dir in $(SUBDIRS); do \
		make -C "$$dir" $@ || exit 1; \
	done

test: test.c

install: $(TARGETS)
	rm -f $(PURPLE_PLUGIN_DIR)/prpltwtr$(PLUGIN_SUFFIX)
	install -m 0755 -d $(PURPLE_PLUGIN_DIR)
	cp prpltwtr$(PLUGIN_SUFFIX) $(PURPLE_PLUGIN_DIR)/prpltwtr$(PLUGIN_SUFFIX)
	for dir in $(SUBDIRS); do \
		make -C "$$dir" $@ || exit 1; \
	done
	for dir in 16 22 48; do \
		(install -m 0755 -d $(PURPLE_PROTOCOL_PIXMAP_DIR)/$$dir && \
		 install -m 0644 data/prpltwtr$$dir.png $(PURPLE_PROTOCOL_PIXMAP_DIR)/$$dir/prpltwtr.png ) \
	done

uninstall: 
	rm -f $(PURPLE_PLUGIN_DIR)/prpltwtr$(PLUGIN_SUFFIX)
	for dir in $(SUBDIRS); do \
		make -C "$$dir" $@ || exit 1; \
	done

clean:
	rm -f $(TARGETS) $(OBJECTS)
	for dir in $(SUBDIRS); do \
		make clean -C "$$dir" $@ || exit 1; \
	done

dist: $(DISTFILES)
	mkdir -p $(PACKAGE)-$(VERSION)
	cp -f $(DISTFILES) $(PACKAGE)-$(VERSION)
	for dir in $(DISTDIRS); do \
		mkdir -p $(PACKAGE)-$(VERSION)/$$dir; \
		find $$dir -maxdepth 1 -type f | xargs cp --target-directory $(PACKAGE)-$(VERSION)/$$dir; \
	done

prpltwtr.tar.gz: dist
	tar cfz $(PACKAGE)-$(VERSION).tar.gz $(PACKAGE)-$(VERSION) 

prpltwtr$(PLUGIN_SUFFIX): $(TWITTER_OBJ)
	$(LD) $(LDFLAGS) -shared $(TWITTER_OBJ) $(LIB_PATHS) $(LIBS) $(DLL_LD_FLAGS) -o prpltwtr$(PLUGIN_SUFFIX).dbgsym
	$(STRIP) -g prpltwtr$(PLUGIN_SUFFIX).dbgsym -o prpltwtr$(PLUGIN_SUFFIX)

prpltwtr.exe: build
	makensis twitter.nsi > /dev/null
