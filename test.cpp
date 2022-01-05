#include "irc.hpp"
#include <stdio.h>
int main () {
	IRC c ("irc.osmarks.net", 6667, "test");
	
	c.set_nick ("kitbot");
	
	c.join_chan ("#b");
	
	IRC::Message h;
	for (;;) {
		int er = c.get_msg (&h);
		//char thing [10];
		//sprintf (thing, "%d", er);
		//if (er != IRC_NOMESSAGE)
		//	c.send_msg (thing, "#b");
	}
	
	return 0;
}
