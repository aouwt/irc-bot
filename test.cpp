#include "irc.hpp"
#include <unistd.h>
#include <string.h>
#include <stdio.h>


#define LEN(arr) (sizeof (arr) / sizeof (arr [0]))

IRC *Servers [16];
size_t ServerCount = -1;


char *lcase (char* str) {
	for (size_t i = 0; str [i]; i ++)
		if (str [i] >= 'A' && str [i] <= 'Z')
			str [i] -= 'A' - 'a';
	return str;
}

bool startswith (const char *str, const char *search) {
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
		IRC *conn;
	} context;
	
	
	
	void ping (context *ctx) {
		ctx -> conn -> send_msg ("kitpong", ctx -> msg -> where);
	}
	
	
	
	
	void help (context *ctx) {
		ctx -> conn -> send_msg ("kitbot -- kit except a bot -- use `kithuh COMMAND` for more information on a command", ctx -> msg -> where);
		ctx -> conn -> send_msg ("commands: kitping, kithelp, kithuh, kitwho", ctx -> msg -> where);
	}
	
	
	
	
	void huh (context *ctx) {
		char _what [16];
		char *what = _what;
		if (sscanf (ctx -> start + 3, "%*[ ]%16s", what) != 1)
			return;
		
		if (! strncmp (what, "kit", 3))
			what = what + 3; // if somebody says kitping as the command then parse it as simply ping
		
		
		if (! strcmp (what, "ping"))
			ctx -> conn -> send_msg ("kitping -- play a game of ping-pong!", ctx -> msg -> where);
		else
		if (! strcmp (what, "help"))
			ctx -> conn -> send_msg ("kithelp -- displays a help screen", ctx -> msg -> where);
		else
		if (! strcmp (what, "huh"))
			ctx -> conn -> send_msg ("kithuh COMMAND -- shows information about a command", ctx -> msg -> where);
		else
		if (! strcmp (what, "who"))
			ctx -> conn -> send_msg ("kitwho [FEILD] -- tells you info about kitbot. FEILD can be `version` or `about` (default)", ctx -> msg -> where);
		else {
			char buf [128];
			snprintf (buf, 128, "unknown command %s", what);
			ctx -> conn -> send_msg (buf, ctx -> msg -> where);
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
		
		ctx -> conn -> send_msg (msg, ctx -> msg -> where);
	}
	
	
	
	void bruh (context *ctx) {
		ctx -> conn -> send_msg ("bruh", ctx -> msg -> where);
	}
	
	
	
	
	void slap (context *ctx) {
		char who [32]; char msg [512]; char act [512];
		if (sscanf (ctx -> start + 4, "%*[ ]%32s", who) != 1)
			return;
		
		#include "kitslap.cpp"
		
		snprintf (act, 512, items [rand () % (LEN (items) - 1)], who);
		snprintf (msg, 512, "\01ACTION %s\x01", act);
		ctx -> conn -> send_msg (msg, ctx -> msg -> where);
	}
		
		
		
}


bool getmsg (kit::context *ctx, IRC::Message *msg) {
	bool eoc = false;
	for (size_t i = 0; i != ServerCount; i ++) {
		switch (Servers [i] -> get_msg (msg)) {
			case IRC_MESSAGE:
				ctx -> conn = Servers [i]; ctx -> msg = msg;
				return true;
				break;
				
			case IRC_NOMESSAGE:
				break;
			default:
				eoc = true;
				break;
		}
	}
	
	if (! eoc)
		sleep (1);
	
	return false;
}




int main (int argc, char *argv []) {
	{
		char *nick = NULL; //(char*) "kitbot";
		
		for (int a = 1; a != argc; a ++) {
			if (argv [a] [0] != '-')
				return -1;
				
			switch (argv [a] [1]) {
				case 's':
					Servers [++ ServerCount] = new IRC (&argv [a] [2], 6667, nick);
					if (Servers [ServerCount] -> err) {
						fputs ("Can't connect!\n", stderr);
						return 1;
					}
					break;
					
				case 'n':
					nick = &argv [a] [2];
					break;
				
				case 'c':
					Servers [ServerCount] -> join_chan (&argv [a] [2]);
					break;
					
				case 'N':
					Servers [ServerCount] -> set_nick (&argv [a] [2]);
					break;
				
				case 'h':
					printf ("Usage: %s [-nHOSTNAME] -sSERVER -cCHANNEL [-NNICK] ...\n", argv [0]);
					return 0;
			}
		}
	}
	
	if (++ ServerCount == 0) {
		fputs ("No server!\n", stderr);
		return -1;
	}
	

	kit::context ctx; IRC::Message msg;
	for (;;) {
		if (getmsg (&ctx, &msg)) {
			char *lstr = lcase (strdup (msg.what));

			char *tok = (char*) strstr (lstr, "kit");
			if (tok == NULL) goto exit;
			else tok += 3;
			
			ctx.start = tok; ctx.str = lstr;
			
			if (startswith (tok, "ping"))
				kit::ping (&ctx);
			else
			if (startswith (tok, "help"))
				kit::help (&ctx);
			else
			if (startswith (tok, "huh"))
				kit::huh (&ctx);
			else
			if (startswith (tok, "who"))
				kit::who (&ctx);
			else
			if (startswith (tok, "bruh"))
				kit::bruh (&ctx);
			else
			if (startswith (tok, "slap"))
				kit::slap (&ctx);
exit:
			free (lstr);
		}
	}
	return 0;
}
