#include "headers/config_server.h"
#include "headers/io_extend.h"

struct sigaction endProg;	// 

/**
 * Exit properly the program, clean semaphores, pipes, ...
 * @param status End status
 */
void exitProgram(int status){
	if(status < SMAIN_PIPE_CONNEXION)  	 unlink(CONNEXION_PIPE);
	if(status < SMAIN_PIPE_MSGRECEPT)    unlink(MSGRECEPT_PIPE);
	if(status < SMAIN_MUT_CONNE)		 mutfree(mutc);
	if(status < SMAIN_MUT_RECEP)		 mutfree(mutw);
	if(status < SMAIN_INIT_CLIENT)		 exit_Client();
	if(status < SMAIN_FORK_RECP) 		 kill(recep_pid,SKILL_SIGNAL);
	if(status < SMAIN_FORK_CONN) 		 kill(conne_pid,SKILL_SIGNAL);
	
	printf("\r*Serv_main *  ## Chat Main server will now exit");
	if(status == SKILL_SIGNAL)	 printf(" : Stopped by user.");
	else if(status != SMAIN_EXIT) printf(" ON ERROR (%d) !\n",status); 
	printf("\n");  		
	exit(status);
}

/**
 * Ping send a message to all client and if they dont anwser kill them ...
 */
void ping() {
	int c;
	int fd_write;
	char buff[100];

    while(1) {
    	sleep(PING_DELAY);
        for(c=0;c<MAX_USER;c++) {
        	if(get_Type(c)!=0) { // ultisateur connecté
        	     if(get_Active(c)==0) {
  					/* Si l'utilisateur à son activité à 0 alors envoi au serveur recept que l'utilisateur 
  					 est déconnecté */
        	     	printf("*Serv_main *    -> Inactivity of %s\n",get_Pseudo(c));
        	     	fd_write = open (MSGRECEPT_PIPE, O_RDWR);
        	     	if ( fd_write == -1) {
						puts ("*Serv_main *    -> Can't open pipe");
					}
						sprintf(buff,"%d:%ld:%d:\n", get_Pid(c),(long int)get_Num(c),DISC);
					// Zone critique : écriture dans le pipe (plusieurs utilisateurs possibles)
					P(mutw);
					if (( write(fd_write,buff, SERVER_RECV_MSG)) == -1) {
						puts ("*Serv_main *    -> /!\\ Can't write into user pipe /!\\");
						close (fd_write);	
						V(mutw);
					}			
  					close (fd_write); 
  					V(mutw); 	
        	     }
        	}			
        }
 		sleep(PING_DELAY);
    	puts("*Serv_main *    -> Ping send !");
    	for(c=0;c<MAX_USER;c++) {
    	    if(get_Type(c)!=0) {
    	    	/* Pour tous les utilisateurs connectés, le serveur envoie un message spécial ping,
    	    	et décrémente l'activité du client, si le client répond son activité est incrémenté
    	    	(serv_recept) sinon son activité arrivera à 0 et il sera déconnecté */
				fd_write = open (get_Pipename(c), O_RDWR);
				if ( fd_write == -1) {
					puts ("*Serv_main *    -> Can't open pipe");
					continue;
				}		

				if (( write(fd_write,":#p\n", SERVER_RECV_MSG)) == -1) {
					puts ("*Serv_main *    -> /!\\ Can't write into user pipe /!\\");
					close (fd_write);	
					continue; 
				}	
				active_dec(c);
  				close (fd_write);
  			}	
		} 
    }
}


/** 
 * Main does : 
 * <ul>
 * <li>Create connexion tube for clients and read entries (fork)</li>
 * <li>Dispose end fonctions</li>
 * <li>...</li>
 * </ul>
 * @return End status
 */
int main(int argc, char ** argv){
	FILE * f;
   	key_t key;
   	// get absolute path and go into
   	char abspath[250];
	getcwd(abspath, (size_t)(250));
	chdir(abspath);
   	
    // pipe folder
   	mkdir(PIPE_FOLDER, 0311);		// only read
   	
   	if(chdir(PIPE_FOLDER) == -1){ 
   		puts("*Serv_main *    -> ERR CHDIR"); 
   		exitProgram(SMAIN_PIPE_FOLDER);
   	}

   	//     F I F O S
	// Create connexion pipe
	if(mkfifo(CONNEXION_PIPE, 0644) == -1){			// wr for server, read for others
        perror("*Serv_main *    -> Can't make connexion pipe, can't continue.");
        exitProgram(SMAIN_PIPE_CONNEXION);
	}
	chmod(CONNEXION_PIPE,0666);
	
	// Create reception pipe
	if(mkfifo(MSGRECEPT_PIPE, 0644) == -1){
        perror("*Serv_main *    -> Can't make reception pipe, can't continue.");
        exitProgram(SMAIN_PIPE_MSGRECEPT);
	}
    chmod(MSGRECEPT_PIPE,0666);
    
    // mutex for pipe used by client
    key = ftok("/etc/magic", 1);
	if( (mutc = mutalloc(key)) == -1){    // CONNEXION_PIPE
		exitProgram(SMAIN_MUT_CONNE);
	}
	key = ftok("/etc/magic", 2);	
	if( (mutw = mutalloc(key)) == -1){    // MSGRECEPT_PIPE
		exitProgram(SMAIN_MUT_RECEP);
	}	

    // Initialize client memory
    if(init_Client() == -1){
        perror("**Serv_main *    -> Can't initialize client memory, can't continue.");
        exitProgram(SMAIN_INIT_CLIENT);    
    }	
    		
    // Users file 
    if((f = fopen(USR_FILE, "r")) == NULL){
        printf("*Serv_main *    -> Can't find users file (USR_FILE='%s'), can't continue.\n", USR_FILE);
        exitProgram(SMAIN_OPEN_USER);
    }	fclose(f);
        		
    
	//     F O R K S
    // Read reception path son
    switch(recep_pid = fork()){
		case -1: perror("*Serv_main *    -> Fork() for reception reader failed. Can't continue.");
			exitProgram(SMAIN_FORK_RECP);
		case 0: 
			listenMsgsPipe();
	}

    // Read connection path (son)
    switch(conne_pid =fork()){
		case -1: perror("*Serv_main *    -> Fork() for connection reader failed. Can't continue.");
			exitProgram(SMAIN_FORK_CONN);
		case 0: 
			connexionReader(recep_pid);
	}

    // handler 
    endProg.sa_handler = exitProgram;
    sigaction(2, &endProg, 0); 
        
    ping();
    
    // ne passe jamais ici ping bloquant		
    while(waitpid(0, 0, 0) <= 0) sleep(1); // seulement 1 fils
    exitProgram(SMAIN_EXIT);
}




