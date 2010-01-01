all: build

ICON = im-twitter.png
ICON_DIR = /usr/share/icons/hicolor/48x48/hildon

PROFILE = twitter.profile
PROFILE_DIR = /usr/share/osso-rtcom

build clean:
	
install :
	install -d $(ICON_DIR)
	for icon in $(ICON); do \
		install -m 0644 $$icon $(ICON_DIR)/$$icon; \
	done
	install -d $(PROFILE_DIR)
	for profile in $(PROFILE); do \
		install -m 0644 $$profile $(PROFILE_DIR)/$$profile; \
	done

uninstall :
	for icon in $(ICON); do \
		rm -f $(ICON_DIR)/$$icon; \
	done
	for profile in $(PROFILE); do \
		rm -f $(PROFILE_DIR)/$$profile; \
	done
	
DISTFILES = $(ICON) $(PROFILE) Makefile
