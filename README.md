### prpltwtr - A Twitter/OSstatus compatible libpurple plugin ###
*Update: in light of twitter's [decision to discourage / forbid new clients](http://www.pcmag.com/article2/0,2817,2381854,00.asp), I'm not sure how much more development will be done.*

Current build status: [![Build Status](https://travis-ci.org/mikeage/prpltwtr.svg?branch=twitter-json)](https://travis-ci.org/mikeage/prpltwtr)

Please see the Changelog file for more information about changes between versions.

This is a [libpurple](http://developer.pidgin.im/wiki/WhatIsLibpurple) \([Pidgin](http://pidgin.im/), [Finch](http://developer.pidgin.im/wiki/Using%20Finch), etc) plugin which treats microblogging sites [Twitter](http://developer.pidgin.im/wiki/Using%20Finch), [identi.ca](http://identi.ca), [status.net](http://status.net/) as IM protocols. 

It treats timelines and searches as chat rooms and the people you follow as buddies. 
You can update your status (posting a message to the home timeline, search, or responding to buddy using an @mention) and send DMs.

  * For support, either post on the [issues page](https://github.com/mikeage/prpltwtr/issues) or contact [@prpltwtr] (http://twitter.com/prpltwtr) or [@mikeage] (http://twitter.com/mikeage)

  * If you receive a lot of tweets with links shortened via bit.ly, t.co, etc, you may find [expand] (https://github.com/mikeage/expand/) to be a useful plugin. Go check it out!

### Compilation Instructions ###
This plugin can be compiled on Linux for either Windows (using mingw) or Linux targets. Compilation on Windows is not currently supported.
For Linux, use the standard:

./autogen.sh ; ./configure ; make ; (sudo) make install

For Windows, unpack the pidgin sources somewhere outside of this plugin, and run:

make -f Makefile.mingw PIDGIN_TREE_TOP=/path/to/where/you/have/pidgin/sources/ installer
