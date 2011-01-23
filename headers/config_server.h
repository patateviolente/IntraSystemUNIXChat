/*****************************************************
 * Server configuration : not shared with client
 * Include shared configuration
 */
#include "config.h"
#include "serv_structs.h"
	
/* ERROR 
 SMAIN_ERROR   1--------80
 SCONN_ERROR   100-----180
 SRECP_ERROR   200-----280
*/ 
#define SMAIN_EXIT 				0
#define SKILL_SIGNAL 			2
#define SMAIN_PIPE_FOLDER 		40	
#define SMAIN_PIPE_CONNEXION 	55
#define SMAIN_PIPE_MSGRECEPT 	54
#define SMAIN_MUT_CONNE			51
#define SMAIN_MUT_RECEP			50
#define SMAIN_INIT_CLIENT 		45
#define SMAIN_OPEN_USER			35
#define SMAIN_FORK_RECP 		29
#define SMAIN_FORK_CONN 		28
#define SMAIN_SON_FAILED 		25
#define SCONN_EXIT 				100
#define SCONN_PIPE_READ 		145
#define SRECP_EXIT 				200
#define SRECP_INIT_CHANNEL 		250
#define SRECP_PIPE_READ 		245

#define PING_DELAY				3		// (seconds)

#define MSG_SERVER 		-1

int recep_pid;  	// initialize by server main (used by smain/sconn)
int conne_pid;	    // initialize by server main (used by smain)

// mutex for pipe (used by client)
int mutc;
int mutw;

/**
 * Exit properly the program, cleanung semaphores, pipes, ...
 * @param status End status
 */
void exitProgram(int status);

void ping();

