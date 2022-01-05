#include "irc.hpp"
#include <fcntl.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


bool startswith (const char *str, const char *search) {
	for (size_t i = 0;; i ++) {
		if (search [i] == '\0') return true;
		if (search [i] != str [i]) return false;
	}
}

IRC::~IRC (void) {}



IRC::IRC (const char* domain, unsigned int port, const char* name) {
	CurBuf = "";
	if (strlen (name) > IRC_NICKLEN) {
		err = IRC_NICKTOOLONG;
		return;
	}
	
	
	struct addrinfo *addr;
	if (getaddrinfo (domain, NULL, NULL, &addr)) {
		err = IRC_DNSERR;
		return;
	}
	
	sockfd = socket (addr -> ai_family, addr -> ai_socktype, addr -> ai_protocol);
	if (sockfd == -1) {
		err = IRC_CANTCONNECT;
		return;
	}

	if (fcntl (sockfd, F_SETFL, O_NONBLOCK)) {
		err = IRC_GENERAL;
		return;
	}

	((struct sockaddr_in*) addr -> ai_addr) -> sin_port = htons (port);
	if (connect (sockfd, addr -> ai_addr, addr -> ai_addrlen)) {
		err = IRC_CANTCONNECT;
		return;
	}

	freeaddrinfo (addr);
	
	char msg [IRC_MESSAGELEN + 2];
	if (snprintf (msg, IRC_MESSAGELEN, "USER %s %s %s %s\r\n", name, name, domain, name) == EOF) {
		err = IRC_TOOLONG;
		return;
	}
	_send (msg);
}




int IRC::_send (const char* what) {
	int sent = 0;
	int len = strlen (what);
	do {
		sent = send (sockfd, what + sent, len - sent, 0);
		if (sent < 0)
			return IRC_GENERAL;
	} while (sent < len);
	
	return IRC_OK;
}





int IRC::set_nick (const char* nick) {
	if (strlen (nick) > IRC_NICKLEN)
		return IRC_NICKTOOLONG;

	char msg [IRC_MESSAGELEN + 2];
	if (snprintf (msg, IRC_MESSAGELEN, "NICK %s\r\n", nick) == EOF)
		return IRC_TOOLONG;

	return _send (msg);
}




int IRC::send_msg (const char* what, const channel_t where) {
	if (strlen (where) > IRC_CHANNELLEN)
		return IRC_CHANTOOLONG;

	char msg [IRC_MESSAGELEN + 2];
	if (snprintf (msg, IRC_MESSAGELEN, "PRIVMSG %s :%s\r\n", where, what) == EOF)
		return IRC_TOOLONG;

	return _send (msg);
}




int IRC::join_chan (const char* ch) {
	if (strlen (ch) > IRC_CHANNELLEN)
		return IRC_CHANTOOLONG;

	char msg [IRC_MESSAGELEN + 2];
	if (snprintf (msg, IRC_MESSAGELEN, "JOIN %s\r\n", ch) == EOF)
		return IRC_TOOLONG;

	return _send (msg);
}


void IRC::_get (void) {
	char buf [IRC_BUFFERLEN];
	ssize_t r = recv (sockfd, buf, sizeof (buf), 0);
	if (r <= 0)
		return;
	buf [r + 1] = '\0';
	std::string s (buf);
	CurBuf += s;
}




int IRC::_getcmd (char* msg) {
	int where = 0;
	
	_get ();

	if ((where = CurBuf.find ("\r\n")) == -1) {
		msg[0] = '\0';
		return IRC_NOMESSAGE;
	}

	
	const char *s = CurBuf.c_str ();
	CurBuf.erase (0, where + 2);

	if (sscanf (s, "%510[^\r]\r\n", msg) == EOF)
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
		return _send ("PONG\r\n");
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
		puts (str);
	}
	
	return IRC_MESSAGE;
			
}
