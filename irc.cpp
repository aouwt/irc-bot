#include "irc.hpp"
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <Socks/ClientSocket.hpp>


bool startswith (const char *str, const char *search) {
	for (size_t i = 0;; i ++) {
		if (search [i] == '\0') return true;
		if (search [i] != str [i]) return false;
	}
}

IRC::~IRC (void) {}



IRC::IRC (const char* domain, unsigned int port, const char* name) {
	socket.setSocket (domain, port);
	
	if (strlen (name) > IRC_NICKLEN) {
		err = IRC_NICKTOOLONG;
		return;
	}
	
	char msg [IRC_MESSAGELEN + 2];
	if (snprintf (msg, IRC_MESSAGELEN, "USER %s %s %s %s\r\n", name, name, domain, name) == EOF) {
		err = IRC_TOOLONG;
		return;
	}
	
	socket.send (msg);
}

int IRC::set_nick (const char* nick) {
	if (strlen (nick) > IRC_NICKLEN)
		return IRC_NICKTOOLONG;

	char msg [IRC_MESSAGELEN + 2];
	if (snprintf (msg, IRC_MESSAGELEN, "NICK %s\r\n", nick) == EOF)
		return IRC_TOOLONG;

	socket.send (msg);
	return IRC_OK;
}

int IRC::send_msg (const char* what, const channel_t where) {
	if (strlen (where) > IRC_CHANNELLEN)
		return IRC_CHANTOOLONG;

	char msg [IRC_MESSAGELEN + 2];
	if (snprintf (msg, IRC_MESSAGELEN, "PRIVMSG %s :%s\r\n", where, what) == EOF)
		return IRC_TOOLONG;

	socket.send (msg);
	return IRC_OK;
}

int IRC::join_chan (const char* ch) {
	if (strlen (ch) > IRC_CHANNELLEN)
		return IRC_CHANTOOLONG;

	char msg [IRC_MESSAGELEN + 2];
	if (snprintf (msg, IRC_MESSAGELEN, "JOIN %s\r\n", ch) == EOF)
		return IRC_TOOLONG;

	socket.send (msg);
	return IRC_OK;
}

int IRC::_getcmd (char* msg) {
	size_t where = 0;
	
	CurBuf += socket.receive ();
	
	if ((where = CurBuf.find ("\r\n")) == 0) {
		msg[0] = '\0';
		return IRC_NOMESSAGE;
	}

	
	const char *s = CurBuf.c_str ();
	CurBuf.erase (0, where + 2);

	if (sscanf (s, ":%510[^\r]s\r\n", msg) == EOF)
		return IRC_PACKETERR;
	
	return IRC_MESSAGE;
}

int IRC::get_msg (IRC::Message *msg) {
	char str [IRC_MESSAGELEN];

retry:
	int erm = _getcmd (str);
	if (erm != IRC_MESSAGE)
		return erm;
	
	if (startswith (str, "PING :"))
		socket.send ("PONG\r\n");
	else
	if (startswith (str, "NOTICE AUTH :"))
		goto retry;
	else
	if (startswith (str, "ERROR :"))
		goto retry;
	else { // privmsg (note: i have no idea how this ensures that)
	//	if (strchr (str, '\x01') != NULL) // note to self -- make this more me-ish
	//		goto retry;
		
		//if (str [0] != ':')
		//	return IRC_PACKETERR;
		//if (strstr (str, "duck") != NULL)
		//	send_msg ("quack", "#b");
		send_msg (str, "#b");
	}
	
	return IRC_MESSAGE;
			
}
