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
		ctx -> conn -> send_msg ("kitbot -- useless irc bot thing -- use `kithuh COMMAND` for more information on a command", ctx -> msg -> where);
		ctx -> conn -> send_msg ("commands: kitping, kithelp, kithuh, kitwho, kitbruh, kitslap, kitecho, kitcopy, kitgame, kitwhisper", ctx -> msg -> where);
	}
	
	
	
	
	void huh (context *ctx) {
		char _what [16];
		char *what = _what;
		if (sscanf (ctx -> start + 3, "%*[ ]%14s", what) != 1)
			return;
		
		if (! strncmp (what, "kit", 3))
			what = what + 3; // if somebody says kitping as the command then parse it as simply ping
		
		
		if (! strcmp (what, "ping"))
			ctx -> conn -> send_msg ("play a game of ping-pong!", ctx -> msg -> where);
		else
		if (! strcmp (what, "help"))
			ctx -> conn -> send_msg ("are you dumb? then use this command!", ctx -> msg -> where);
		else
		if (! strcmp (what, "huh"))
			ctx -> conn -> send_msg ("...", ctx -> msg -> where);
		else
		if (! strcmp (what, "who"))
			ctx -> conn -> send_msg ("kitwho [FIELD] -- tells you info about yours truly. FIELD can be `version`, `about`, or left blank", ctx -> msg -> where);
		else
		if (! strcmp (what, "bruh"))
			ctx -> conn -> send_msg ("bruh.", ctx -> msg -> where);
		else
		if (! strcmp (what, "slap"))
			ctx -> conn -> send_msg ("kitslap [VICTIM] -- i will slap you or somebody else (if i feel like it)", ctx -> msg -> where);
		else
		if (! strcmp (what, "copy"))
			ctx -> conn -> send_msg (ctx -> msg -> what, ctx -> msg -> where);
		else
		if (! strcmp (what, "echo"))
			ctx -> conn -> send_msg ("says something", ctx -> msg -> where);
		else
		if (! strcmp (what, "game"))
			ctx -> conn -> send_msg ("play a game to see who can jump off a bridge fastest!", ctx -> msg -> where);
		else
		if (! strcmp (what, "whisper"))
			ctx -> conn -> send_msg ("shhh", ctx -> msg -> where);
		else {
			char buf [128];
			snprintf (buf, 128, "unknown command %s", what);
			ctx -> conn -> send_msg (buf, ctx -> msg -> where);
		}
	}
	
	
	
	
	void who (context *ctx) {
		char what [16]; char msg [512];
		if (sscanf (ctx -> start + 3, "%*[ ]%14s", what) != 1)
			what [0] = '\0';
		
		if (what [0] == '\0')
			snprintf (msg, 510, "hi im kitbot. you can call me kitbot. i am underwhelmingly utterly undeniably useless.");
		else
		if (! strcmp (what, "version"))
			snprintf (msg, 510, "kitbot version idk_i_dont_keep_track (codename \"Dumbass\"). built on %s", __DATE__);
		else
		if (! strcmp (what, "about"))
			snprintf (msg, 510, "kitbot -- a useless irc bot. https://github.com/aouwt/irc-bot");
		else
			snprintf (msg, 510, "unknown thingy %s", what);
		
		ctx -> conn -> send_msg (msg, ctx -> msg -> where);
	}
	
	
	
	void bruh (context *ctx) {
		ctx -> conn -> send_msg ("bruh", ctx -> msg -> where);
	}
	
	
	
	
	void slap (context *ctx) {
		char _who [32]; char *who = _who;
		char msg [512];
		if (sscanf (ctx -> start + 4, "%*[ ]%30s", who) != 1)
			who = ctx -> msg -> who;
		
		#include "kitslap.hpp"
		
		snprintf (msg, 510, items [rand () % (LEN (items) - 1)], who);
		ctx -> conn -> send_action (msg, ctx -> msg -> where);
	}
		
	
	void bot (context *ctx) {
		ctx -> conn -> send_msg ("what do you want", ctx -> msg -> where);
	}
	
	
	void special (context *ctx) {
		if (strcmp (lcase (ctx -> msg -> who), "kit")) {
			ctx -> conn -> send_msg ("do i know you?", ctx -> msg -> where);
			return;
		}
		
		char what [16]; char arg [64];
		if (sscanf (ctx -> start + 7, "%*[ ]%14s %62s", what, arg) < 1)
			return;
		
		if (! strcmp (what, "suicide"))
			exit (0);
		else
		if (! strcmp (what, "join"))
			ctx -> conn -> join_chan (arg);
	}
	
	
	
	
	void echo (context *ctx) {
		ctx -> conn -> send_msg (ctx -> start + 5, ctx -> msg -> where);
	}
	
	void copy (context *ctx) {
		ctx -> conn -> send_msg (ctx -> msg -> what, ctx -> msg -> where);
	}
	
	void whisper (context *ctx) {
		ctx -> conn -> send_action ("whispers in your ear", ctx -> msg -> who);
	}
	
	void game (context *ctx) {
		ctx -> conn -> send_msg ("let's play a game: who can jump off the bridge fastest", ctx -> msg -> where);
		sleep (1);
		ctx -> conn -> send_msg ("kit won", ctx -> msg -> where);
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
		char *nick = (char*) "kitbot";
		
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
			else
			if (startswith (tok, "bot"))
				kit::bot (&ctx);
			else
			if (startswith (tok, "special"))
				kit::special (&ctx);
			else
			if (startswith (tok, "echo"))
				kit::echo (&ctx);
			else
			if (startswith (tok, "copy"))
				kit::copy (&ctx);
			else
			if (startswith (tok, "game"))
				kit::game (&ctx);
			else
			if (startswith (tok, "whisper"))
				kit::whisper (&ctx);
exit:
			free (lstr);
		}
	}
	return 0;
}
