This is a [libpurple](http://developer.pidgin.im/wiki/WhatIsLibpurple) [Pidgin](http://pidgin.im/), [Finch](http://developer.pidgin.im/wiki/Using%20Finch), etc plugin which treats microblogging sites [Twitter](http://developer.pidgin.im/wiki/Using%20Finch), [identi.ca](http://identi.ca), [status.net](http://status.net/) as IM protocols. 

It treats timelines and searches as chat rooms and the people you follow as buddies. 

You can update your status (posting a message to the home timeline, search, or responding to buddy using an @mention) and send DMs.

If you don't like this plugin, I recommend trying out [microblog-purple](http://code.google.com/p/microblog-purple/) or [pidgin-twitter](http://www.honeyplanet.jp/pidgin-twitter/) (I am not affiliated with either)

*Update: in light of twitter's [decision to discourage / forbid new clients](http://www.pcmag.com/article2/0,2817,2381854,00.asp), I'm not sure how much more development will be done.*

  * For support, either post on the [issues page](https://github.com/mikeage/prpltwtr/issues) or contact [@prpltwtr] (http://twitter.com/prpltwtr) or [@mikeage] (http://twitter.com/mikeage)

  * If you receive a lot of tweets with links shortened via bit.ly, t.co, etc, you may find [expand] (https://github.com/mikeage/expand/) to be a useful plugin. Go check it out!

### Version 0.13.0 (in progress) ###
  * Add Japanese translation
  * Add support for Twitter API v1.1 (restore functionality)
  * Fix Debian packaging

### Version 0.12.0 ###
  * Fix character count and add colors
  * Fix crash if attempting to RT, favorite, or delete while disconnected
  * Add followers as they tweet, rather than on connection (the followers.xml endpoint has been deprecated). Because of this change (which changed the storage mechanism somewhat), I *STRONGLY* encourage you to delete all your twitter buddies and let prpltwtr add them again. But it should migrate automatically. Hopefully.
  * Detect when a user changes his name; preserve history, etc.

### Version 0.11.0 ###
  * Make some errors recoverable. Prpltwtr should try to reconnect, and not disable the account. I know this covers the case of sleep on Windows; hopefully others as well
  * Load user info dynamically when requesting info for a user not in the buddy list
  * Fix multipage requests for home timeline and lists. More than 200 tweets should now be received (up to 800 for twitter.com, depending on the value set in the account preferences). Also fix a race condition where the timer refreshes could start before the initial retreive finished (much more critical after the multipage fix)
  * Add c:\PortableApps\PortableApps to the installer
  * Tell the plugin(s) that we support offline messages (b/c twitter does, obviously)

### Version 0.10.0 ###
  * Lists now include subscribed lists, not just your own
  * Use per-account proxy settings instead of the global proxy settings
  * Fix for the Win32 plugins depending on the .dbgsym version, not the .dlls themselves

### Version 0.9.0 ###
  * Several crashes fixed 
  * Identi.ca work. Twitter requires OAuth (no longer an option); status.net sites can use either. Note that you'll need to register your own custom application if you want to use OAuth with status.net sites and provide the consumer key and secret in the account options.
  * Split the prpltwtr plugin into three: libprpltwtr for the infrastructure, libprpltwtr_twitter for Twitter, and libprpltwtr_statusnet for identi.ca, etc. You may need to manually delete the old versions (but I hope not)
  * Let the user choose the default alias. Suggested by @space_tiger. If you already added your friends as buddies, the fastest thing to do is delete the group and let prpltwtr re-add them after changing this option.
 * *NOTE FOR LINUX USERS: YOU MUST MANUALLY DELETE THE OLD `*`prpltwtr.so FILES FROM YOUR PURPLE PLUGIN DIRECTORY. THE NAMES CHANGED IN THIS VERSION (I HOPE THIS WILL BE THE LAST TIME I DO THIS TO YOU!) *
 * *NOTE FOR ALL USERS: IT APPEARS THAT YOU MAY NEED TO RE-ACTIVATE AND RE-CONFIGURE THE GTKPRPLTWTR PLUGIN. PLEASE LET ME KNOW IF YOU SEE THIS. THANKS *

### Version 0.8.0 ###
  * Improved buddy info (you can also now click on a buddy name in a tweet)
  * Switch application name PrplTwtr2 (will require re-authorizing using OAuth). This is because of [https://groups.google.com/group/twitter-development-talk/browse_thread/thread/e954fc0f8b5aa6ec Twitter's decision to add a new permission level for DMs].

### Version 0.7.0 ###
  * *NOTE FOR LINUX USERS: YOU MUST MANUALLY DELETE THE OLD prpltwtr.`*` FILES FROM YOUR PIDGIN'S PLUGIN DIRECTORY. THEY WILL NOW BE INSTALLED TO THE libpurple DIRECTORY*
  * Make the list and timeline refresh intervals user set parameters
  * Fix some memory leaks
  * Fix infinite loop when "replying to all" if there's an @ without a name after it (a mistake, or a 4sq checking tweet)
  * Add --without-pidgin option to ./configure for building just prpltwtr and not gtkprpltwtr. 
  * Basic favorites support. The action menu is NOT updated, though, after favoriting / unfavoriting a tweet.
  * Add some useful links to the plugin option menu

### Version 0.6.4 ###
  * Better error prints.
  * Search: don't retrieve all old tweets when rejoining a search
  * Search: customize whether the search text is included, and where (prepend, append, neither)
  * Show rate limit status as the topic for chats and timelines
  * L10n; currently only a Spanish translation is available. Volunteers welcome!
  * List support! Not tested extensively, and probably broken for Haze / Empathy, but seems to work fairly well in Pidgin. Some code cleanup still needs to be done.

### Version 0.6.3 ###
  * Fix for make install (Linux only)

### Version 0.6.2 ###
  * "Conversations"
  * Report spammers
  * API fix for missing tweets

### Version 0.6.1 ###
  * Proper 64 bit support
  * "Normal" make system.
  * General code cleanup

### Version 0.6.0 ###
  * See the [Changelog] for a full summary

### Version 0.5.2 ###
  * Fixes crash caused by bad interaction with some protocols/plugin combinations

### Version 0.5.1 ###
  * Separated pidgin only functionality into a separate plugin, gtkprpltwtr. It is automatically enabled for pidgin users.
  * Several bug fixes. 

### Version 0.5.0 ###
  * Added Twitter Actions (Retweet, reply, delete, etc) (Pidgin 2.6+ only)
  * Added icons to chat converstions (Pidgin only)
  * Added character counter in while typing (Pidgin only)
  * Fixed duplicate IMs when sending IM
  * Fixed formatting when receiving tweets with HTML
  * Fixed links for status.net accounts
  * Now working for Pidgin 2.5 (with less features)

### Version 0.4.0 ###
  * Renamed to prpltwtr
  * Added proper icon (Purple Twitter Icon)
  * Added long post functionality, to automatically split messages that are too long
  * Status.Net support
  * Added OAuth support (Default to off, this will change)
  * Added option to set buddies as offline after X (default 24) hours

### Version 0.3.0 ###
  * Added partial url handling (#search can be clicked for new search) (Pidgin 2.6+ only)
  * Added options to auto-open timelines/searches when new tweets arrive.
  * Added option to send DMs and options on how to treat ims (default to DM or @mention)
  * Fixed telepathy support

### Version 0.2.0 ###
  * Home Timeline added (acts like a chat room)
  * Searches added (also acts like a chat room)
  * Lots of fixes and updates
