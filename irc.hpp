#include <stdio.h>
#include <Socks/ClientSocket.hpp> // https://github.com/j-Sans/Socks


#define IRC_OK 0
#define IRC_GENERAL 1
#define IRC_TOOLONG 2
#define IRC_NOCHAN 3
#define IRC_CANTCONNECT 4

#define IRC_MESSAGELEN 512
class IRC {
	public:
		typedef char nick_t [10];
		typedef char channel_t [201];
		
		typedef struct {
			char msg [IRC_MESSAGELEN + 2];
			nick_t from;
			channel_t* channel;
		} Message;
		
		IRC (const char* domain, unsigned int port, const char* name);
		IRC (const char* domain, const char* name) { IRC (domain, 6667, name); };
		~IRC (void);
		
		int get_msg (Message* msg);
		int send_msg (channel_t* channel, const char* msg);
		
		int join_channel (const char* ch);
		int set_nick (const char* nick);
		
		int is_connected (void);
	
	private:
		ClientSocket socket;
		int err = 0;
		
		int send (const char* what);
};
