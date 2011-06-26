EXTRA_DIST = \
	Makefile.mingw

pkgdir = $(PIDGIN_PLUGINDIR)

PRPLTWTR_PATH=../prpltwtr/

GTKPRPLTWTR_SOURCES = \
	gtkprpltwtr.c \
	gtkprpltwtr.h \
	gtkprpltwtr_prefs.c \
	gtkprpltwtr_prefs.h \
	gtkprpltwtr_charcount.c \
	gtkprpltwtr_charcount.h \
	gtkprpltwtr_convicon.c \
	gtkprpltwtr_convicon.h

AM_CFLAGS = \
	$(st)

libgtkprpltwtr_la_LDFLAGS = \
	-module \
	-avoid-version 

pkg_LTLIBRARIES = \
	libgtkprpltwtr.la

libgtkprpltwtr_la_SOURCES = $(GTKPRPLTWTR_SOURCES)

AM_CPPFLAGS = \
	$(DEBUG_CFLAGS) \
	$(PIDGIN_CFLAGS) \
	$(PIDGIN_PLUGINS) \
	-I$(PRPLTWTR_PATH) \
	-DLOCALEDIR=\"$(LIBPURPLE_DATADIR)/locale\"

AM_LDFLAGS = \
	-L$(PRPLTWTR_PATH) \
	-lprpltwtr
