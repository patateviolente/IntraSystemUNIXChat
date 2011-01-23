/**********************************************
 * Shared configuration (server & client)
 **/
#include <stdio.h>		// . . .
#include <stdlib.h>		// . . .
#include <unistd.h>		// 
#include <sys/types.h>	//
#include <sys/stat.h>	//
#include <string.h>		// strcat(), str...
#include <fcntl.h>		//
#include <time.h>		// time()
#include <signal.h>     // sigaction
#include <math.h> 		// pow generate number 

#define CONNEXION_PIPE 			"connexion_pipe"
#define MSGRECEPT_PIPE 			"recept_pipe"
#define MAX_PSEUDO_LENGTH 		16	// Maximum chars in pseudonyms
#define MAX_LOGIN_LENGTH 		16  // 15+1 Maximum chars in a login
#define MAX_PWD_LENGTH			16  // 15+1 Maximum chars in a password
#define MAX_NUMGEN				9	// 10 +1 Maximum chars in a generated number
#define MAX_NUMPIPE				9	// 10 +1 Maximum chars in a pipe number
#define MAX_PIDLENGTH 			7	// 7  +1 Maximum chars in a pid
#define MAX_CHANNEL_LENGTH		21	// 20 +1 Maximum chars in a channel
#define MAX_MSG_LENGTH			250	// 450+1 Maximum chars by posts
#define PIPE_FOLDER				"chatpipes/"		// WARNING: absolute path if server/client executed as a command !
#define USR_FILE				"../users"		// attention si relatif, depuis le dossier pipes ! ex: /home/ju/admin/chat_ctrl/users
												// DEFAULT : "../users"

#define MAXARGNAME_LEN 10
#define MAXARGVALUE_LEN 20


// COLORS : {30=black ; 31=red ; 32=green ; 33=yellow ; 34=blue ; 35=magenta ; 36=cyan ; 37=white }
//      BG: {40=black ; 41=red ; 42=green ; 43=yellow ; 44=blue ; 45=magenta ; 46=cyan ; 47=white }
//           0=standard text   ;   1=bold    ;    7=reverse    ;    5=blink
#define colorize(p) printf("\033[%sm",p)

// pipe_name		LOGIN + NUM(pipe) 
#define MAX_PIPE_NAME			MAX_LOGIN_LENGTH+MAX_NUMPIPE

// option length  when is a char*  = X\0
#define MAX_OPTION   	2

/* message buffer */
// ask connexion				LOGIN:PASSWORD:PID:NUM(pipe)\n 
#define CONNECT_ASK_MSG			MAX_LOGIN_LENGTH+MAX_PWD_LENGTH+MAX_PIDLENGTH+MAX_NUMPIPE+1
// answer connexion 			PSEUDO:NUM(generate)\n 
#define CONNECT_ANS_MSG			MAX_PSEUDO_LENGTH+MAX_NUMGEN+1
// send message 				PID:NUM(generate):[OPTION]:[MESSAGE]\n 
#define SERVER_SEND_MSG			MAX_PIDLENGTH+MAX_NUMGEN+MAX_OPTION+MAX_MSG_LENGTH+1
// receive message 				[PSEUDOemet]:[PSEUDOdest]:MESSAGE\n 
#define SERVER_RECV_MSG			MAX_PSEUDO_LENGTH+MAX_PSEUDO_LENGTH+MAX_MSG_LENGTH+1	

// option
#define MSG      0 
#define CREATE   1
#define JOIN     2
#define KICK     3
#define WHISPER  4
#define DISC  	 5
#define PING     6


// server msg for liste client
#define RESET_LC  "#*"
#define JOIN_LC   "#+"
#define LEAVE_LC  "#-"	



