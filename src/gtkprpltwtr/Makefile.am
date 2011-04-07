EXTRA_DIST = \
	Makefile.mingw

pkgdir = $(PIDGIN_PLUGINDIR)

PRPLTWTR_PATH=../prpltwtr

GTKPRPLTWTR_SOURCES = \
gtkprpltwtr.c \
gtkprpltwtr.h \
gtkprpltwtr_prefs.c \
gtkprpltwtr_prefs.h \
twitter_charcount.c \
twitter_charcount.h \
twitter_convicon.c \
twitter_convicon.h

PRPLTWTR_SOURCES =  \
$(PRPLTWTR_PATH)/twitter_prefs.c \
$(PRPLTWTR_PATH)/twitter_request.c \
$(PRPLTWTR_PATH)/twitter_api.c \
$(PRPLTWTR_PATH)/twitter_search.c \
$(PRPLTWTR_PATH)/twitter_util.c \
$(PRPLTWTR_PATH)/twitter_xml.c \
$(PRPLTWTR_PATH)/twitter_endpoint_chat.c \
$(PRPLTWTR_PATH)/twitter_endpoint_search.c \
$(PRPLTWTR_PATH)/twitter_endpoint_timeline.c \
$(PRPLTWTR_PATH)/twitter_endpoint_list.c \
$(PRPLTWTR_PATH)/twitter_buddy.c \
$(PRPLTWTR_PATH)/twitter_conn.c \
$(PRPLTWTR_PATH)/twitter_endpoint_im.c \
$(PRPLTWTR_PATH)/twitter_endpoint_dm.c \
$(PRPLTWTR_PATH)/twitter_endpoint_reply.c \
$(PRPLTWTR_PATH)/twitter_mbprefs.c 

AM_CFLAGS = \
	$(st)

gtkprpltwtr_la_LDFLAGS = \
	-module \
	-avoid-version \
	$(PIDGIN_LIBS)

pkg_LTLIBRARIES = \
	gtkprpltwtr.la

gtkprpltwtr_la_SOURCES = \
	$(GTKPRPLTWTR_SOURCES) \
	$(PRPLTWTR_SOURCES)

AM_CPPFLAGS = \
	$(DEBUG_CFLAGS) \
	$(PIDGIN_CFLAGS) \
	-DLOCALEDIR=\"$(PIDGIN_DATADIR)/locale\" \
	-I../prpltwtr/
