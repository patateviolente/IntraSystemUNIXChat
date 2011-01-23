#include "../headers/client_chat.h"
#include "../headers/ipctools.h"
#include <signal.h>

struct sigaction sigexit;
key_t key1, key2, key3;
pid_t pid_listener;
int fd_read;
void capture(){}
static int mutw;	// mutex to write in msg pipe


/** Last function **/
void exitChat(){
	sendMessage(DISC, "");	// DISConnect
	if(o_gui) endwin();		// end curses mode
	
	// Fin écouteur
	close(fd_read);
	kill(pid_listener, SIGKILL);
	printf(" > N'écoute plus le serveur.\n", pid_listener);
	
	// désallocation
	shmfree(key1);
	shmfree(key2);
	free(msgcut);
	
	puts("Fin d'exécution du client de chat.");
	exit(0);
}	

/** 
 * Interface of chat, call other package functions to display and send messages.
 * Open GUI if true.
 */
void clientChat(){
	// shared memory
	key1 = ftok("/etc/wgetrc", getpid());
	key2 = ftok("/etc/timezone", getpid());
	key3 = ftok("/etc/magic",2);
	text = (Shared*) shmalloc(key1, sizeof(Shared));
	users = (Users*) shmalloc(key2, sizeof(Users));
	msgcut = (MsgCut*) malloc(sizeof(MsgCut));
	
	// Initialise msg struct
	text->pos = text->tours = 0;
	strcpy(text->buf[text->pos], TXT_NO_MESSAGE);
	strcpy(users->chan, "Public");
	
	// init mutex : w in pipe
	if( (mutw = mutalloc(key3)) == -1){
		puts("Erreur mutalloc mutw(7)");
		exitChat();
	}
	
	// repaint signal
	sig_newmsg.sa_handler = capture;
	sigaction(NEW_MESSAGE, &sig_newmsg, 0); 
    
	// exit signal
	sigexit.sa_handler = &exitChat;
	if(o_gui) 	sigaction(2, &sig_newmsg, 0);
	else 		sigaction(2, &sigexit, 0);
	
	// Reading process
	switch(pid_listener = fork()){
		case -1 : perror("fork() failed"); exit(-1);
		case 0:
			listenChat();
			puts("FIN DE L'ECOUTE");
			exit(0);
			break;
	}
	
	// Display prompt chat field (open GUI, integrated field , if activ !)
	if(o_gui){
		printf("Open Graphic User Interface\n", o_gui);
		initInterface();		// block
	} else
		promptField();			// block
}


/** 
 * <p>Listen `pipe_name` for text and display it.
 * Stop only when pipe destroyed, return nothing.</p> 
 */
int listenChat(){
	
	char recvBuf[SERVER_RECV_MSG];
	int i;
	
	if (( fd_read = open(pipename, O_RDWR)) == -1) {
			perror ("*Error opening message reception pipe"); 
			exit(80); 
	}
	
	begin:
	while(1)
	{
		// READ LINE
		if ( read(fd_read, recvBuf, SERVER_RECV_MSG) != -1){
			
			// client in/out ? ...
			if(strncmp(recvBuf, ":#", 2) == 0){
				switch(recvBuf[2]){
					case '+' : addClient(recvBuf); break;
					case '-' : removeClient(recvBuf); break;
					case '*' : removeAllClient();
						// get channel name
						i=3;
						while(recvBuf[i] != '\n')
							users->chan[i-3] = recvBuf[i++];
						users->chan[i-3] = '\0';
					break;
					case 'p' : sendMessage(PING,""); goto begin;	// skip repaint
					default: break;
				}
				
				if(!o_gui)
					printf("New user join : %s", recvBuf);
				else
					kill(getppid(), NEW_MESSAGE);
			}
			// ... or message ?
			else{
				// store into array
				if(text->pos >= MAX_MSG_STORED-1){
					text->pos = -1;
					text->tours++;
				}
				if(text->pos == 0 && strcmp(text->buf[0], TXT_NO_MESSAGE) == 0) text->pos--;	// erase first line (no message)
				strcpy(text->buf[++text->pos], recvBuf);
				
				if(!o_gui){		// direct display
					cutMessage(recvBuf);
					if(o_colors) colorize("31"); 
					colorize("1");
					printf ("%s : ", msgcut->pseudo);
					if(o_colors)colorize("34");
					printf ("%s", msgcut->msg);
					colorize("0");	puts("");
				}
				else{
					//V(repnt);
					kill(getppid(), NEW_MESSAGE);
				}
			}
		}
	}
	close(fd_read);		// to reopen it again with block adapter
	
}

/** 
 * Capture message and send it to server.
 */
int promptField(){
	char buff[ MAX_OPTION+MAX_MSG_LENGTH];

	while(1){
		buff[0]='\0';
		readKeyboard(buff, MAX_OPTION+MAX_MSG_LENGTH);
		sendMessage(MSG,buff);
	}	
	return 0;
}

/** 
 * Send message on reception pipe, decrypt argument 
 */
int sendMessage(int option,char buff[]){
	int fd_write, n;
	char msg[MAX_MSG_LENGTH];
	char sendBuf[SERVER_SEND_MSG];
	char option2[MAX_OPTION];
	option2[0] = '\0';
	
	char c;
	int i=1, k=-1;
	
	// OPTION (without GUI)
	if(buff[0]=='/') {		// option type /xxx
		c = buff[i];
		while(c != ' ' && c != '\0' && i<MAX_OPTION) {
			option2[i-1] = c;
			c = buff[++i];
		}
		option2[i-1] = '\0';
		printf("option *%s*\n", option2);
		
		c = buff[++i];
		while( c != '\0' && i<MAX_MSG_LENGTH) {
			msg[k++]=c;
			c=buff[i++];
		}
		msg[k]='\0' ;

		option = atoi(option2);

	}
	else	// standard msg
		strcpy(msg,buff);

	
	if (( fd_write = open (MSGRECEPT_PIPE, O_WRONLY)) == -1) {
		perror("Can't open recept pipe\n"); return 1; }
	sprintf(sendBuf,"%d:%ld:%d:%s\n", pid, num, option, msg);
	strcpy(option2,"");
	
	// Zone critique : écriture dans le pipe (plusieurs utilisateurs possibles)
	P(mutw);
	if (( n = write(fd_write,sendBuf,SERVER_SEND_MSG) ) == -1) {	
		printf ("Can't write to server"); V(mutw);return 1; }
	
	close (fd_write);
	V(mutw);
	// Fin zone critique
	
	return 0;
}


void callRepaint(){
	if(o_gui) repaint();
}


