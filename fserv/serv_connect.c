#include "../headers/serv_connect.h"

extern struct sigaction endProg;


/* en cas d'erreur ou fin de programme serv_conne*/
void exitConnet(int status) {
	printf("\r*Serv_conne*  ## Connection server will now exit");
	if(status == SKILL_SIGNAL)		printf(", serveur main asks me to die.\n",status);
	else if(status != SCONN_EXIT)	printf(" ON ERROR (%d) !\n",status);
	exit ((status == SCONN_EXIT) ? 0 : status);
}


/**
 * Will analyse readed buffer at connexion.
 * Add the client into plugged user list.
 * Send message back to client.
 */
void connexionProcess(char * line){
	char 		pseudo		[MAX_PSEUDO_LENGTH];
	char 		login		[MAX_LOGIN_LENGTH];
	char 		password	[MAX_PWD_LENGTH];
	char 		pipe_num	[MAX_NUMPIPE];
	short int 	type;
	int 		pid;
	long int 	num;
	char 		pipe_name	[MAX_PIPE_NAME];
	char 		buff_write	[CONNECT_ANS_MSG];
	int 		fd_write;
	int			init;
	// parse line
	if(parseConnexionAsk(line,login,password,&pid,pipe_num)){
		puts("*Serv_conne*    -> User ejected, can't parse connexion phrase.");
		return;
	}
	
	// create the pipe_name
	strcpy(pipe_name, login);
	strcat(pipe_name, pipe_num);
	
	// check user ( login and password) and get his pseudo and type (admin or client)
    type = isAllowedUser(login,SuperFastHash(password, strlen(password)),pseudo);
    if( type == -1){
		printf("*Serv_conne*    ->  /!\\ L'identification de %s a échouée !\n", login);
		return;
	}
	
	
	num = (long int)(rand()/(float)RAND_MAX * pow(10,(MAX_NUMGEN-1))); 
	printf("*Serv_conne*    -> User detected login=%s, pwd=%s, pid=%d,num=%s \n",login,password,pid,pipe_num);
		
	//chdir(PIPE_FOLDER);
	if(mkfifo(pipe_name, 0622) == -1)		// WR for serv (and same users' client) + read for others
        printf("*Serv_conne*    -> Server can't create pipe \"%s\" for user %s", pipe_name, pseudo);
	else
	{
		printf("*Serv_conne*    -> Making pipe \"%s\"\n",pipe_name);
		// prévient le client :
		// opening pipe
		if ( (fd_write = open(pipe_name, O_WRONLY)) == -1) {
			puts ("*Serv_conne*    -> Can't open pipe");
			unlink(pipe_name);
			return;
		}
		
		// on crée le client
		printf("*Serv_conne*    -> New client... Initialization of \"%s\" ",pseudo);	
		init = new_Client_connect(pipe_name,pseudo,pid,num,type);
   		if(init == 0)
			printf("succes !\n");
		else {
			printf("failed !\n");
			unlink(pipe_name);
			return;
    	}
    
        printf("*Serv_conne*    -> Send num=%ld, pseudo=%s\n",num,pseudo);
		// writing into pipe
		sprintf(buff_write, "%ld:%s:\n", num, pseudo);		// sprintf(buff_write, "%ld:%s:%d\n", num, pseudo);	
		if(( write(fd_write, buff_write, CONNECT_ANS_MSG)) == -1) {
			puts ("*Serv_conne*    -> /!\\ Can't write into user pipe /!\\");
			return;
		}
		
		// on détruit le pipe si le client n'a pas été crée
		if(init != 0) {
			unlink(pipe_name);
		}
		close(fd_write);
	}
}


/**
 * The connexion reader should always be occuped by a process. When a client
 * is writing a log sentence, the CR will verify into the log file to 
 * authentificate.
 */
void connexionReader(){
	int fd_read;
	char buffC[CONNECT_ASK_MSG];

	printf("*Serv_conne* START\n");
	
	// initialize rand for generate num for client
	srand(getpid()); 
	
	endProg.sa_handler = exitConnet; 		   // demande du père de die  (ici + ctrl c)
 	sigaction(SKILL_SIGNAL, &endProg, 0);     // demande du père de die  (ici + ctrl c)

	puts("*Serv_conne* ... Now reading pipe ..."); 
	
	if ((fd_read = open(CONNEXION_PIPE, O_RDWR)) == -1) { // O_RDWR : for make a block read
		perror ("*Serv_conne*    -> Error opening connexion pipe");  
		exitConnet(SCONN_PIPE_READ);
	}
		
	while(1)
	{ 
		// reading a connexion line : pseudo
		if ( read(fd_read, buffC, CONNECT_ASK_MSG) != -1){
			printf ("*Serv_conne*    << read  %s", buffC);
			connexionProcess(buffC);
		}
	}
	close(fd_read);

	puts("*Serv_conne*    END OF CONNEXION PIPE READER");
	exitConnet(SCONN_EXIT);
}



int parseConnexionAsk(char * str, char * login, char * password,int* pid, char * pipe_num){
	int i=0, u=0,k=0,r=0;
	char pid_char[MAX_PIDLENGTH];
	
	while(str[i] != ':'){
		if(i == MAX_LOGIN_LENGTH) return -1;
		login[i] = str[i];
		i++;
	}
	login[i] = '\0';
	i++;
	while(str[i] != ':'){
		if(u == MAX_PWD_LENGTH) return -1;
		password[u] = str[i];
		i++;u++;
	}	
	password[u] = '\0';
	i++;
	while(str[i] != ':'){
		if(k == 6) return -1;
		pid_char[k] = str[i];
		i++;k++;
	}	
	pid_char[k]='\0';
	*pid = atoi (pid_char);
	i++;
	
	while(str[i] != '\n'){
		if(r == MAX_NUMPIPE) return -1;
		pipe_num[r] = str[i];
		i++; r++;
	}
	pipe_num[r] = '\0';
	return 0;
}


/** <p>Vérifie qu'un utilisateur est autorisé à intégrer le chat en vérifiant
 * que son login est entré avec le bon mot de passe hashé correspondant.</p>
 * <p>Le fichier doit être structuré de la façon suivant :<br ./>
 * login:motdepassehashémd5:pseudo:rang\n
 * autrelogin:autremotdepasse:autrepseudo:autrerang\n
 * ...</p>
 * @param login Login
 * @param mdp mot de passe déjà hashé
 * @param pseudo pseudo de l'utilisateur
 * @return rang de l'utilisteur, -1 s'il n'existe pas ou déja connecté.
 *  **/
int isAllowedUser(char* login, int32_t mdpin,char* pseudo){
	FILE* f;
	char rlogin[MAX_LOGIN_LENGTH];
	char rmdp[64], mdp[64];
	char line [MAX_LOGIN_LENGTH+64+MAX_PSEUDO_LENGTH+1];
	int i,j, u, rang;
	int32_t mdptmp;

	// open file
	if( (f = fopen(USR_FILE, "r")) == NULL ){
		printf("*Serv_conne*    -> /!\\ Erreur : le fichier de la liste des utilisateurs introuvable.\n");
		return -1;
	}
	
	// read line by line
	read:
	while( fgets(line, sizeof(line), f) != NULL){

		i = j = u = 0;

		//printf("*Serv_conne*    -> Line ={%s}\n",line);
		
		while(line[i] != ':')		// copy login
			rlogin[i] = line[i++];
		rlogin[i] = '\0';
		i++;
		while (line[i] != ':')		// copy mdp
			rmdp[u++] = line[i++];
		rmdp[u] = '\0';
		mdptmp = atoi(rmdp);
		i++;
		while (line[i] != ':')		// copy pseudo
			pseudo[j++] = line[i++];
		pseudo[j] = '\0';
		
		rang = line[++i] - 48;		// convert char en int 
		
		//printf("*Serv_conne*    -> Login={%s}    Mdp={%s}   Pseudo={%s}    Rang={%d}\n", rlogin, rmdp,pseudo, rang);
		
		if(rang != 1 && rang != 2) continue;	// invalid
		
		if((strcmp(login, rlogin) != 0) 	// login !=   ou   mdp !=
			|| mdpin != mdptmp) continue;
		
		// trouvé !
		fclose(f);	
		if(verifPseudo(pseudo)!=-1) // déja connecté
			return -1;
		return rang;
	}
	
	fclose(f);
	return -1;
}
