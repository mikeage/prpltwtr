EXTRA_DIST = \
	Makefile.mingw

pkgdir = $(PURPLE_PLUGINDIR)

PRPLTWTR_SOURCES = \
	defaults.h \
	prpltwtr_api.c \
	prpltwtr_api.h \
	prpltwtr_buddy.c \
	prpltwtr_buddy.h \
	prpltwtr.c \
	prpltwtr_conn.c \
	prpltwtr_conn.h \
	prpltwtr_endpoint_chat.c \
	prpltwtr_endpoint_chat.h \
	prpltwtr_endpoint_dm.c \
	prpltwtr_endpoint_dm.h \
	prpltwtr_endpoint_im.c \
	prpltwtr_endpoint_im.h \
	prpltwtr_endpoint_reply.c \
	prpltwtr_endpoint_reply.h \
	prpltwtr_endpoint_search.c \
	prpltwtr_endpoint_search.h \
	prpltwtr_endpoint_timeline.c \
	prpltwtr_endpoint_timeline.h \
	prpltwtr_endpoint_list.c \
	prpltwtr_endpoint_list.h \
	prpltwtr.h \
	prpltwtr_mbprefs.c \
	prpltwtr_mbprefs.h \
	prpltwtr_prefs.c \
	prpltwtr_prefs.h \
	prpltwtr_request.c \
	prpltwtr_request.h \
	prpltwtr_search.c \
	prpltwtr_search.h \
	prpltwtr_util.c \
	prpltwtr_util.h \
	prpltwtr_xml.c \
	prpltwtr_xml.h 

AM_CFLAGS = \
	$(st)

libprpltwtr_twitter_la_LDFLAGS = -module -avoid-version $(PURPLE_LIBS)

libprpltwtr_statusnet_la_LDFLAGS = -module -avoid-version $(PURPLE_LIBS)

st = 

pkg_LTLIBRARIES = \
	libprpltwtr.la \
	libprpltwtr_twitter.la \
	libprpltwtr_statusnet.la

libprpltwtr_la_SOURCES = $(PRPLTWTR_SOURCES)
libprpltwtr_la_LIBADD = $(GLIB_LIBS)

libprpltwtr_twitter_la_SOURCES = prpltwtr_plugin_twitter.c prpltwtr_plugin.h
libprpltwtr_twitter_la_LIBADD = libprpltwtr.la

libprpltwtr_statusnet_la_SOURCES = prpltwtr_plugin_statusnet.c prpltwtr_plugin.h
libprpltwtr_statusnet_la_LIBADD = libprpltwtr.la

AM_CPPFLAGS = \
	$(DEBUG_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(PURPLE_CFLAGS) \
	$(PURPLE_PLUGINS) \
	-DLOCALEDIR=\"$(LIBPURPLE_DATADIR)/locale\"
