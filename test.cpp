#include "irc.hpp"
#include <stdio.h>
int main () {
	IRC c ("ubq323.website", 6667, "kitbot");

	if (c.err) return c.err;
	puts ("connect");
	
	c.set_nick ("kitbot");
	
	c.join_chan ("#b");

	puts ("done");
	IRC::Message msg;
	for (;;) {
		int e = c.get_msg (&msg);
		if (e == IRC_MESSAGE)
			c.send_msg (msg., msg.who);
		else
		if (e != IRC_NOMESSAGE)
			printf ("err: %i\n", e);
	}
	
	return 0;
}
