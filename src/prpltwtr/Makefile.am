EXTRA_DIST = \
	Makefile.mingw

pkgdir = \
	$(PURPLE_PLUGINDIR)

PRPLTWTR_SOURCES = \
defaults.h \
twitter_api.c \
twitter_api.h \
twitter_buddy.c \
twitter_buddy.h \
twitter.c \
twitter_conn.c \
twitter_conn.h \
twitter_endpoint_chat.c \
twitter_endpoint_chat.h \
twitter_endpoint_dm.c \
twitter_endpoint_dm.h \
twitter_endpoint_im.c \
twitter_endpoint_im.h \
twitter_endpoint_reply.c \
twitter_endpoint_reply.h \
twitter_endpoint_search.c \
twitter_endpoint_search.h \
twitter_endpoint_timeline.c \
twitter_endpoint_timeline.h \
twitter_endpoint_list.c \
twitter_endpoint_list.h \
twitter.h \
twitter_mbprefs.c \
twitter_mbprefs.h \
twitter_prefs.c \
twitter_prefs.h \
twitter_request.c \
twitter_request.h \
twitter_search.c \
twitter_search.h \
twitter_util.c \
twitter_util.h \
twitter_xml.c \
twitter_xml.h 


AM_CFLAGS = \
	$(st)

prpltwtr_la_LDFLAGS = \
	-module \
	-avoid-version \
	$(PURPLE_LIBS)

pkg_LTLIBRARIES = \
	prpltwtr.la

prpltwtr_la_SOURCES = \
	$(PRPLTWTR_SOURCES)

AM_CPPFLAGS = \
	$(DEBUG_CFLAGS) \
	$(PURPLE_CFLAGS) \
	$(PURPLE_PLUGINS) \
	-DLOCALEDIR=\"$(LIBPURPLE_DATADIR)/locale\"
