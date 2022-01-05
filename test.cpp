#include "irc.hpp"
#include <stdio.h>
int main () {
	IRC c ("ubq323.website", 6667, "kitbot");

	if (c.err) return c.err;
	puts ("connect");
	
	c.set_nick ("kitbot");
	
	c.join_chan ("#b");

	puts ("done");
	IRC::Message h;
	for (;;) {
		int er = c.get_msg (&h);
		//char thing [10];
		//sprintf (thing, "%d", er);
		//if (er != IRC_NOMESSAGE)
		//	c.send_msg (thing, "#b");
//		printf ("%i\n", er);
	}
	
	return 0;
}
