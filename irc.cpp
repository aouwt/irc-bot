#include "irc.hpp"
#include <stdio.h>
#include <Socks/ClientSocket.hpp>

IRC::IRC (const char* domain, unsigned int port, const char* name) {
	
}

int IRC::send (const char* what) {
	char msg [IRC_MESSAGELEN + 2];
	if (snprintf (msg, IRC_MESSAGELEN, "%s\n\r", what) == EOF)
		return IRC_TOOLONG;
	
	
}

int IRC::set_nick (const char* nick) {
	
}
