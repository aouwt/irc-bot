#include <stdlib.h>
#include <stdio.h>
#include <string>


#define IRC_OK	0	// No errors
#define IRC_GENERAL	1	// General error

#define IRC_NICKTOOLONG	10	// Too long nick
#define IRC_MSGTOOLONG	11	// Too long message
#define IRC_CHANTOOLONG	12	// Too long channel name
#define IRC_TOOLONG	15	// Too long total message

#define IRC_PACKETERR	20	// Malformed incoming packet
#define IRC_DNSERR	21
#define IRC_CANTCONNECT	22


#define IRC_NOMESSAGE	0
#define IRC_MESSAGE	-1



#define IRC_MESSAGELEN	512
#define IRC_NICKLEN	9
#define IRC_CHANNELLEN	50
#define IRC_BUFFERLEN	IRC_MESSAGELEN * 2
class IRC {
	public:
		typedef char nick_t [IRC_NICKLEN + 2];
		typedef char channel_t [IRC_CHANNELLEN + 2];
		
		typedef struct {
			char msg [IRC_MESSAGELEN + 2];
			//nick_t from;
			//channel_t* channel;
		} Message;
		int err = 0;
		
		IRC (const char* domain, unsigned int port, const char* name);
		IRC (const char* domain, const char* name) { IRC (domain, 6667, name); };
		~IRC (void);
		
		int get_msg (Message* msg);
		int send_msg (const char* what, const channel_t where);
		
		int join_chan (const char* ch);
		int set_nick (const char* nick);
		
		int is_connected (void);
	
	private:
		std::string CurBuf;
		int sockfd = 0;
		
		int _getcmd (char* msg);
		int _send (const char* what);
		void _get (void);
};
