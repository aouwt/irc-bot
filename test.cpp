#include "irc.hpp"
#include <unistd.h>
#include <string.h>
#include <stdio.h>

IRC c ("ubq323.website", 6667, "kitbot");

char* lcase (char* str) {
	for (size_t i = 0; str [i]; i ++)
		if (str [i] >= 'A' && str [i] <= 'Z')
			str [i] -= 'A' - 'a';
	return str;
}

bool strstartswith (const char *str, const char *search) {
        for (size_t i = 0;; i ++) {
                if (search [i] == '\0') return true;
                if (search [i] != str [i]) return false;
        }
}



namespace kit {
	typedef struct {
		IRC::Message *msg;
		char *start;
		char *str;
	} context;
	
	void ping (context *ctx) {
		c.send_msg ("kitpong", ctx -> msg -> where);
	}
	
	void help (context *ctx) {
		c.send_msg ("kitbot -- kit except a bot -- use `kithuh COMMAND` for more information on a command", ctx -> msg -> where);
		c.send_msg ("commands: kitping, kithelp, kithuh, kitwho", ctx -> msg -> where);
	}
	
	void huh (context *ctx) {
		char _what [16];
		char *what = _what;
		if (sscanf (ctx -> start + 3, "%*[ ]%16s", what) != 1)
			return;
		
		if (! strncmp (what, "kit", 3))
			what = what + 3; // if somebody says kitping as the command then parse it as simply ping
		
		
		if (! strcmp (what, "ping"))
			c.send_msg ("kitping -- play a game of ping-pong!", ctx -> msg -> where);
		else
		if (! strcmp (what, "help"))
			c.send_msg ("kithelp -- displays a help screen", ctx -> msg -> where);
		else
		if (! strcmp (what, "huh"))
			c.send_msg ("kithuh COMMAND -- shows information about a command", ctx -> msg -> where);
		else
		if (! strcmp (what, "who"))
			c.send_msg ("kitwho [FEILD] -- tells you info about kitbot. FEILD can be `version` or `about` (default)", ctx -> msg -> where);
		else {
			char buf [128];
			snprintf (buf, 128, "unknown command %s", what);
			c.send_msg (buf, ctx -> msg -> where);
		}
	}
	
	void who (context *ctx) {
		char what [16]; char msg [512];
		if (sscanf (ctx -> start + 3, "%*[ ]%16s", what) != 1)
			return;
		
		if (! strcmp (what, "version"))
			snprintf (msg, 512, "kitbot version yes");
		else
		if (! strcmp (what, "about"))
			snprintf (msg, 512, "kitbot -- a useless irc bot. https://github.com/aouwt/irc-bot");
		else
			return;
	}
}




int main () {

	if (c.err) return c.err;
	puts ("connect");
	
	c.set_nick ("kitbot");
	
	c.join_chan ("#b");

	puts ("done");
	IRC::Message msg;
	for (;;) {
		int e = c.get_msg (&msg);
		if (e == IRC_MESSAGE) {
			char *lstr = lcase (strdup (msg.what));

			char *tok = (char*) strstr (lstr, "kit");
			if (tok == NULL) goto exit;
			else tok += 3;
			
			kit::context ctx;
			ctx.msg = &msg; ctx.start = tok; ctx.str = lstr;

			if (strstartswith (tok, "ping"))
				kit::ping (&ctx);
			else
			if (strstartswith (tok, "help"))
				kit::help (&ctx);
			else
			if (strstartswith (tok, "huh"))
				kit::huh (&ctx);
exit:
			free (lstr);
		} else
		if (e == IRC_NOMESSAGE)
			sleep (1);
	}
	
	return 0;
}
