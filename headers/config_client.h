/*****************************************************
 * Server configuration : not shared with server
 * Include shared configuration
 */
#include "config.h"
#define NEW_MESSAGE 8					// signal
#define MAX_MSG_STORED 500				//
#define MAX_USR_STORED 200
#define TXT_NO_MESSAGE "Public::No messages"


/** Variables defined during connexion procedure : **/
/* Each client have a pipe and choice themselve their name composed by the 
 * login ended by a succin number other client shouldn't know ! */
char 		pipe_num[MAX_NUMPIPE]; 		// User own succint number (char)
long int  	pipe_number;				//     ""       ""    ""  (as int) 
int  		pid;						// pid of client
long int  	num;						// number generate by server (for security)
char 		pseudo[MAX_PSEUDO_LENGTH];	// pseudonym
char 		pipename[MAX_PIPE_NAME];	// pipe name
int 		type;
pid_t		pid_listener;



/** PARAMETERS **/
int o_gui;			// graphical user interface
int o_guicolors;	// colors for gui
int o_colors;		// colors for console
struct sigaction sig_newmsg;

// stock messages
typedef struct {
	char buf[MAX_MSG_STORED][MAX_MSG_LENGTH];
	int pos, tours;
} Shared;
Shared* text;

// analyse (one) msg
typedef struct {
	char chan[MAX_CHANNEL_LENGTH];
	char pseudo[MAX_PSEUDO_LENGTH];
	char msg[MAX_MSG_LENGTH];
} MsgCut;
MsgCut * msgcut;

// stock connected users
typedef struct {
	char pseudo[MAX_USR_STORED][MAX_PSEUDO_LENGTH];
	int color[MAX_USR_STORED];
	int nb;
	char chan[MAX_CHANNEL_LENGTH+1];
} Users;
Users * users;

