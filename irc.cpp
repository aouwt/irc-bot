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
#include <errno.h>


static bool startswith (const char *str, const char *search) {
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
retry:
	if (connect (sockfd, addr -> ai_addr, addr -> ai_addrlen)) {
		switch (errno) {
			case EINPROGRESS: case EALREADY:
				goto retry;
			default:
				err = IRC_CANTCONNECT;
				return;
		}
	}

	freeaddrinfo (addr);
	
	char msg [IRC_MESSAGELEN + 2];
	if (snprintf (msg, IRC_MESSAGELEN, "USER %s %s %s %s\r\nNICK %s\r\n", name, name, domain, name, name) == EOF) {
		err = IRC_TOOLONG;
		return;
	}
	_send (msg);
}



int IRC::_send (const char* what) {
	#ifdef IRC_DEBUG
		printf ("\n<%s", what);
	#endif
	int sent = 0; int erc;
	int len = strlen (what);
	do {
		erc = send (sockfd, what + sent, len - sent, 0);
		if (erc >= 0)
			sent += erc;
		else
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


int IRC::send_action (const char* what, const channel_t where) {
	if (strlen (where) > IRC_CHANNELLEN)
		return IRC_CHANTOOLONG;

	char msg [IRC_MESSAGELEN + 2];
	if (snprintf (msg, IRC_MESSAGELEN, "PRIVMSG %s :\01ACTION %s\01\r\n", where, what) == EOF)
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


int IRC::_get (void) {
	char buf [IRC_BUFFERLEN];
	ssize_t r = recv (sockfd, buf, sizeof (buf), 0);
	
	if (r == 0) return 0;
	else
	if (r < 0) {
		if (errno == EWOULDBLOCK)
			return 0;
		else
			return errno + 1000;
	}
	
	buf [r] = '\0';
	#ifdef IRC_DEBUG
		printf ("\n>%s", buf);
	#endif
	std::string s (buf);
	CurBuf += s;
	return 0;
}




int IRC::_getcmd (char* msg) {
	size_t where = 0;
	
	int e = _get ();
	if (e)
		return e;

	if ((where = CurBuf.find ("\r\n")) == std::string::npos)
		return IRC_NOMESSAGE;

	
	const char *s = CurBuf.c_str ();

	e = sscanf (s, "%510[^\r]\r\n", msg);
	CurBuf.erase (0, where + 2);
	if (e == EOF)
		return IRC_PACKETERR;
	
	return IRC_MESSAGE;
}





int IRC::get_msg (IRC::Message *msg) {
	char str [IRC_MESSAGELEN];

retry:
	int erm = _getcmd (str);
	if (erm != IRC_MESSAGE)
		return erm;
	
	
	if (startswith (str, "PING :")) {
		char ping [512];
		char pong [512];
	
		if (sscanf (str, "PING :%[^\r]\r\n", ping) != 1)
			return IRC_PACKETERR;
		
		if (snprintf (pong, IRC_MESSAGELEN, "PONG :%s\r\n", ping) == EOF)
			return IRC_TOOLONG;
		return _send (pong);
	} else
	
	
	if (startswith (str, "NOTICE AUTH :"))
		goto retry;
		
		
		
	else
	if (startswith (str, "ERROR :"))
		goto retry;
		
		
		
	else
	if (str [0] == ':') { // privmsg
		
		if (sscanf (str, ":%9[^! ]%*[^ ]%*[ ]PRIVMSG%*[ ]%50[^ ]%*[ ]:%510[^\r]\r\n", msg -> who, msg -> where, msg -> what) != 3)
			return IRC_PACKETERR;
		
		return IRC_MESSAGE;
	}		
	return IRC_NOMESSAGE;
}
