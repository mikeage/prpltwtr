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

#define TWITTER_PREF_REPLIES_TIMEOUT_NAME "twitter_replies_timeout"
#define	TWITTER_PREF_REPLIES_TIMEOUT_DEFAULT 60

#define TWITTER_PREF_USER_STATUS_TIMEOUT_NAME "twitter_friends_timeout"
#define	TWITTER_PREF_USER_STATUS_TIMEOUT_DEFAULT (60 * 5)
