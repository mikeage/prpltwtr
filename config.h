#ifndef G_GNUC_NULL_TERMINATED
#  if __GNUC__ >= 4
#    define G_GNUC_NULL_TERMINATED __attribute__((__sentinel__))
#  else
#    define G_GNUC_NULL_TERMINATED
#  endif /* __GNUC__ >= 4 */
#endif /* G_GNUC_NULL_TERMINATED */

#ifdef _WIN32
#	include <win32dep.h>
#endif


/* The number of tweets to return per page, up to a max of 100 */
#define TWITTER_SEARCH_RPP_DEFAULT 20

#define TWITTER_INITIAL_REPLIES_COUNT 150
#define TWITTER_EVERY_REPLIES_COUNT 40

#define TWITTER_PROTOCOL_ID "prpl-twitter"

/* We'll handle TWITTER_URI://foo as an internal action */
#define TWITTER_URI "putter"
