#include "headers/config_server.h"
#include "headers/io_extend.h"

struct sigaction endProg;	// 


/** SEE AT BOTTOM */
void decryptArgs(int argc, char ** argv);


/**
 * Exit properly the program, clean semaphores, pipes, ...
 * @param status End status
 */
void exitProgram(int status){
	if(status < SMAIN_PIPE_CONNEXION)  	 unlink(CONNEXION_PIPE);
	if(status < SMAIN_PIPE_MSGRECEPT)    unlink(MSGRECEPT_PIPE);
	if(status < SMAIN_INIT_CLIENT)		 exit_Client();
	if(status < SMAIN_FORK_RECP) 		 kill(recep_pid,SKILL_SIGNAL);
	if(status < SMAIN_FORK_CONN) 		 kill(conne_pid,SKILL_SIGNAL);
	
	printf("\r*Serv_main *  ## Chat Main server will now exit");
	if(status == 2)	 printf(" : Stopped by user.");
	else if(status != SMAIN_EXIT) printf(" ON ERROR (%d) !\n",status); 
	printf("\n");  		
	exit(status);
}

/*
void ping() {
	int c;
	int fd_write;
    while(1) {
        c=0;
        while(c<MAX_USER) {
        	if(get_Type(c)!=0) {
        	     if(get_Active(c)==0) {
        	          // printf("je kill %s\n",get_Pipename(c));
        	     	 // kill_Client(c);
        	     	// puts("j'ai kill");
        	     }
        	}
        	c++;			
        }
 		sleep(5);
    	puts("*Serv_main * Ping send !");
    	c=0;
    	while(c<MAX_USER) {
    	    if(get_Type(c)!=0) {
				fd_write = open (get_Pipename(c), O_RDWR);
					printf("je passe avec %d\n",fd_write);
				if ( fd_write == -1) {
					puts ("*Serv_recep* Can't open pipe\n");
					continue;
				}		

				if (( write(fd_write,"!ping\n", SERVER_RECV_MSG)) == -1) {
					puts ("*Serv_recep*  /!\\ Can't write into user pipe /!\\");
					close (fd_write);	
					continue; 
				}	
				active_dec(c);
  				close (fd_write);
  			}	
  			c++;
		} 
		sleep(1);
    }
}*/


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
   	decryptArgs(argc, argv);
   	
   	// get absolute path and go into
   	char abspath[250];
	getcwd(abspath, (size_t)(250));
	chdir(abspath);
   	
    // pipe folder
   	mkdir(PIPE_FOLDER, 0200);		// only read
   	chmod(PIPE_FOLDER, 200);
   	if(chdir(PIPE_FOLDER) == -1) puts("ERR CHDIR");
   	
   	//     F I F O S
	// Create connexion pipe
	if(mkfifo(CONNEXION_PIPE, 0644) == -1){			// wr for server, read for others
        perror("*Serv_main * Can't make connexion pipe, can't continue.");
        exitProgram(SMAIN_PIPE_CONNEXION);
	}

	// Create reception pipe
	if(mkfifo(MSGRECEPT_PIPE, 0644) == -1){
        perror("*Serv_main * Can't make reception pipe, can't continue.");
        exitProgram(SMAIN_PIPE_MSGRECEPT);
	}
    
    // Initialize client memory
    if(init_Client() == -1){
        perror("*Serv_main * Can't initialize client memory, can't continue.");
        exitProgram(SMAIN_INIT_CLIENT);    
    }	
    
    // Users file 
    if((f = fopen(USR_FILE, "r")) == NULL){
        printf("*Serv_main * Can't find users file (USR_FILE='%s'), can't continue.\n", USR_FILE);
        exitProgram(SMAIN_INIT_CLIENT);
    }	fclose(f);
        		
    
	//     F O R K S
    // Read reception path son
    switch(recep_pid = fork()){
		case -1: perror("*Serv_main * Fork() for reception reader failed. Can't continue.");
			exitProgram(SMAIN_FORK_RECP);
		case 0: 
			listenMsgsPipe();
	}

    // Read connection path (son)
    switch(conne_pid =fork()){
		case -1: perror("*Serv_main * Fork() for connection reader failed. Can't continue.");
			exitProgram(SMAIN_FORK_CONN);
		case 0: 
			connexionReader(recep_pid);
	}

    // handler 
    endProg.sa_handler = exitProgram;
    sigaction(2, &endProg, 0); 
     
     
    //ping();
    			
    while(waitpid(0, 0, 0) <= 0) sleep(1); // seulement 1 fils
    exitProgram(SMAIN_EXIT);
}


/** Ajoute un utilisateur au fichier users 
 * Vérifie qu'il n'existe pas déjà.
 * L'entrée doit être du type user:motdepasse:rang avec rang un numéro 
 * entre 1 et 2, et le  mot de passe non déjà hashé.
 */
void addUser(char * l){
	FILE* f;
	char line [64+MAX_PSEUDO_LENGTH+1];

	puts("ADD USER ");
	
	// open file
	if( (f = fopen("users", "r")) == NULL ){
		printf(" /!\\ Erreur : le fichier de la liste des utilisateurs inouvrable.\n");
		return;
	}
	
	// read line by line
	while( fgets(line, sizeof(line), f) != NULL){
		printf("%s", line);
	}
	
	fclose(f);
	exit(0);
}


/**
 * Apply args.
 * @argc Number of arguments, included program name
 * @argv Args array
 */

void decryptArgs(int argc, char ** argv){
	int eq, len, i;
	char nom[MAXARGNAME_LEN], value[MAXARGVALUE_LEN];
	char c;
	char * rec;
	char tmp[50];
	
	// inititialisations
	// ... default option values
	
	
	for(i=1; i<argc; i++){		// all args
		len = strlen(argv[i]);
		// option type -x
		if(len == 2 && argv[i][0] == '-'){
			switch(argv[i][1]){
				case 'a': 
					// ADD USER
					if(i+1 < argc)
					addUser(argv[i+1]);
					exit(0);
				case 'r': 
					// REMOVE USER
					puts("ADD"); 
					exit(0);
			}
		}
	}
	
}

