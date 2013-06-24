EXTRA_DIST = \
	Makefile.mingw

pkgdir = $(PURPLE_PLUGINDIR)

PT_SOURCES = \
				   defines.h \
				   pt.h \
				   pt.c 

AM_CFLAGS = \
	$(st)

libpt_la_LDFLAGS = -module -avoid-version $(PURPLE_LIBS)

st = 

pkg_LTLIBRARIES = \
	libpt.la 

libpt_la_SOURCES = $(PT_SOURCES)
libpt_la_LIBADD = $(GLIB_LIBS)

AM_CPPFLAGS = \
	$(DEBUG_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(PURPLE_CFLAGS) \
	$(PURPLE_PLUGINS) \
	-DLOCALEDIR=\"$(LIBPURPLE_DATADIR)/locale\"
