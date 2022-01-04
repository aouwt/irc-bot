#include "irc.hpp"
#include <stdio.h>
#include <ClientSocket.hpp>

IRC::IRC (const char* domain, unsigned int port, const char* name) {
	if (socket.setSocket (domain, port)) {
		err = IRC_CANTCONNECT;
		return;
	}
}

int IRC::set_nick (const char* nick) {
	if (strlen (nick) > IRC_NICKLEN)
		return IRC_NICKTOOLONG;

	char msg [IRC_MESSAGELEN + 2];
	if (snprintf (msg, IRC_MESSAGELEN, "NICK %s\r\n", nick) == EOF)
		return IRC_MSGTOOLONG;

	socket.send (msg);
}

int IRC::send_msg (const char* what, const channel_t where) {
	if (strlen (where) > IRC_CHANNELLEN)
		return IRC_CHANTOOLONG;

	char msg [IRC_MESSAGELEN + 2];
	if (snprintf (msg, IRC_MESSAGELEN, "PRIVMSG %s :%s\r\n", where, what) == EOF)
		return IRC_TOOLONG;

	socket.send (msg);
}

int IRC::join_chan (const char* ch) {
	if (strlen (ch) > IRC_CHANNELLEN)
		return IRC_CHANTOOLONG;

	char msg [IRC_MESSAGELEN + 2];
	if (snprintf (msg, IRC_MESSAGELEN, "JOIN %s\r\n", ch) == EOF)
		return IRC_TOOLONG;

	socket.send (msg);
}
