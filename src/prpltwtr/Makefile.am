EXTRA_DIST = \
	Makefile.mingw

pkgdir = $(PURPLE_PLUGINDIR)

PRPLTWTR_SOURCES = \
				   pt.h \
				   pt.c 

AM_CFLAGS = \
	$(st)

libprpltwtr_twitter_la_LDFLAGS = -module -avoid-version $(PURPLE_LIBS)

libprpltwtr_statusnet_la_LDFLAGS = -module -avoid-version $(PURPLE_LIBS)

st = 

pkg_LTLIBRARIES = \
	libprpltwtr.la 

libprpltwtr_la_SOURCES = $(PRPLTWTR_SOURCES)
libprpltwtr_la_LIBADD = $(GLIB_LIBS)

AM_CPPFLAGS = \
	$(DEBUG_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(PURPLE_CFLAGS) \
	$(PURPLE_PLUGINS) \
	-DLOCALEDIR=\"$(LIBPURPLE_DATADIR)/locale\"
