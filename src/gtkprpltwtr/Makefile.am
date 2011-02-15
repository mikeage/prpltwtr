EXTRA_DIST = \
	Makefile.mingw

pkgdir = $(PIDGIN_PLUGINDIR)

GTKPRPLTWTR_SOURCES = \
gtkprpltwtr.c \
gtkprpltwtr.h \
gtkprpltwtr_prefs.c \
gtkprpltwtr_prefs.h \
twitter_charcount.c \
twitter_charcount.h \
twitter_convicon.c \
twitter_convicon.h 

AM_CFLAGS = \
	$(st)

gtkprpltwtr_la_LDFLAGS = \
	-module \
	-avoid-version \
	$(PIDGIN_LIBS)

pkg_LTLIBRARIES = \
	gtkprpltwtr.la

gtkprpltwtr_la_SOURCES = \
	$(GTKPRPLTWTR_SOURCES)

AM_CPPFLAGS = \
	$(DEBUG_CFLAGS) \
	$(PIDGIN_CFLAGS) \
	-DLOCALEDIR=\"$(LIBPURPLE_DATADIR)/locale\" \
	-I../prpltwtr/
