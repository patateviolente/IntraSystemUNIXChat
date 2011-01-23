#include "../headers/serv_recept.h"

/** Read continuously reception pipe, verify the member authorisation. 
 * Display it to the chanel he is registered.
 */
static struct sigaction newClient;
extern struct sigaction endProg;

/** 
 * En cas d'erreur ou fin de programme serv_recep
 */
void exitRecep(int status) { 
	if(status<SRECP_INIT_CHANNEL) exit_Channel();
	printf("\r*Serv_recep*  ## Reception server will now exit");
	// pour le ctrl-c ou demande du père de die SKILL_SIGNAL=2
	if(status==SKILL_SIGNAL) 	printf(", serveur main asks me to die.",status);
	else if(status!=SRECP_EXIT) printf(" ON ERROR (%d) !",status);
	printf("\n");
	exit((status == SRECP_EXIT) ? 0 : status);
}

void init_Client_Channel() {
    char msg[MAX_MSG_LENGTH];
    int id_client;
	printf("*Serv_recep*    -> Client... Initialization channel ");
	// initialise le dernier client et récupère son id
	id_client = new_Client_recept();
    if(id_client >= 0) {
   		printf("succes !\n");		
   		sprintf(msg,"Welcome in the channel %s.",get_Name(get_Canal(id_client)));
		sendMessage(msg,MSG_SERVER,id_client,NULL); 
							 
		sprintf(msg,"%s has joined the channel.",get_Pseudo(id_client));  
		sendMessage(msg,MSG_SERVER,-1,get_Canal(id_client)); 
		sendListClient(msg,id_client); 
   		}
	else
		printf("failed !\n");
}

void listenMsgsPipe(){
	int fd_read;
	char buffC[SERVER_SEND_MSG];
	
	printf("*Serv_recep* START\n");

    // handler pour un nouveau client de serv_conn
    newClient.sa_handler = init_Client_Channel;
    sigaction(15, &newClient, 0);
    
    // demande du père de die (ctrl c)
    endProg.sa_handler = exitRecep; 
    sigaction(SKILL_SIGNAL, &endProg, 0);    
    
    // Initialize channel
    if(init_Channel() == -1){
		perror("*Serv_recept*    -> Can't initialize memory for channels, can't continue.");	
        exitRecep(SRECP_INIT_CHANNEL);      
    }
 
	puts("*Serv_recep* ... Now reading pipe ...");
	
	if (( fd_read = open(MSGRECEPT_PIPE, O_RDWR )) == -1) { // O_RDWR : for make a block read
		perror ("*Serv_recep*    -> Error opening message reception pipe");  
		exitRecep(SRECP_PIPE_READ);
	} 
				
	while(1)
	{
		// reading a message
		if ( read(fd_read, buffC, SERVER_SEND_MSG) != -1){
			printf ("*Serv_recep*    << read  %s", buffC);
			messageProcess(buffC);
		}
	}
	
	close(fd_read);
	
	puts("*Serv_recep* END OF MSGRECEPT PIPE READER"); 
	exitRecep(SRECP_EXIT);    
}

void messageProcess(char * line){
	int pid;
	long int num;
    int option;
	char msg[MAX_MSG_LENGTH];
	int id_client;
	int fd_write;
	char buff_write[SERVER_RECV_MSG];
	int error;
	
	// message mal formé
	if(parseMessageSend(line,&pid,&num,&option,msg)){
		puts("*Serv_recep*    -> Message ejected, can't parse message send.");
		return;
	}

	// client non trouvé association (pid / num ) non valide
	if((id_client = get_Id_Client(pid,num))==-1) {
		puts("*Serv_recep*    -> Message ejected, client not found.");
		return;
	}
    
    Liste_Channel old_channel = get_Canal(id_client);

	switch(option) {
		case MSG :
				sendMessage(msg,id_client,-1,get_Canal(id_client));
				return;
		case CREATE :
				error = new_Channel(msg, id_client);
				switch (error) {
					case  0 : printf("*Serv_recep*    -> Client %s create channel %s \n", get_Pseudo(id_client),msg);
							 /*-1- Envoi message "client left c" sur l'ancien channel du client + maj liste clients des autres clients du channel*/
							 /*-1-*/ sprintf(msg,"%s has left channel.",get_Pseudo(id_client));  
							 /*-1-*/ sendMessage(msg,MSG_SERVER,-1,old_channel); 
							 /*-1-*/ delListClient(msg ,id_client,old_channel);
							  
							 /*-2- Envoi message "Welcome in the channel c" au client concerné*/
							 /*-2-*/ sprintf(msg,"Welcome in the channel %s.",get_Name(get_Canal(id_client)));
							 /*-2-*/ sendMessage(msg,-1,id_client,NULL); 
							  
							 /*-3- Envoi la liste des clients au client concerné */
							 /*-3-*/ sendListClient(msg ,id_client); break;
					case -1 : printf("*Serv_recep*    -> /!\\ Client %s can't create channel ( channel already exists) \n", get_Pseudo(id_client)); 
							  sendMessage("Channel already exists",-1,id_client,NULL); break;
					case -2 : printf("*Serv_recep*    -> /!\\ Client %s can't create channel ( no memory for channel) \n", get_Pseudo(id_client)); 
							  sendMessage("There is to many channels",-1,id_client,NULL); break;
					case -3 : printf("*Serv_recep*    -> /!\\ Client %s can't create channel ( no memory for client ) \n", get_Pseudo(id_client)); 
							  sendMessage("No memory for client",-1,id_client,NULL); break;
				}
				return;
        case JOIN :
				error = join_Channel(msg, id_client);
				switch (error) {
					case  0 : printf("*Serv_recep*    -> Client %s joined channel %s \n", get_Pseudo(id_client),msg);
						      /*-1- Envoi message "client left c" sur l'ancien channel du client + maj liste clients des autres clients du channel*/
							 sprintf(msg,"%s has left channel.",get_Pseudo(id_client));  
							 sendMessage(msg,MSG_SERVER,-1,old_channel); 
							 delListClient(msg ,id_client,old_channel);
							 
							 /*-2- Envoi message "Welcome in the channel c" au client concerné*/
							 sprintf(msg,"Welcome in the channel %s.",get_Name(get_Canal(id_client)));
							 sendMessage(msg,MSG_SERVER,id_client,NULL); 
							 
							 /*-3- Envoi message "client joined" sur le nouveau channel et lui envoie la liste des clients + maj liste clients des autres clients du channel*/
							 sprintf(msg,"%s has joined the channel.",get_Pseudo(id_client));  
							 sendMessage(msg,MSG_SERVER,-1,get_Canal(id_client)); 
							 sendListClient(msg,id_client); 
							  break;
					case -1 : printf("*Serv_recep*    -> /!\\ Client %s can't joined channel ( channel not found ) \n", get_Pseudo(id_client)); 
							  sendMessage("Channel not found.",MSG_SERVER,id_client,NULL); 
							  break;
					case -2 : printf("*Serv_recep*    -> /!\\ Client %s can't joined channel ( Client already in this channel ) \n", get_Pseudo(id_client)); 
							  sendMessage("You are already in this channel",MSG_SERVER,id_client,NULL); 
							  break;
					case -3 : printf("*Serv_recep*    -> /!\\ Client %s can't joined channel ( no memory for client in this channel ) \n", get_Pseudo(id_client)); 
							  sendMessage("There is to many clients in this channel",MSG_SERVER,id_client,NULL); 
							  break;
				}
        		return;
        case KICK :
        		error = kick(msg, id_client);	
				switch (error) {
					case  -1 : printf("*Serv_recep*    ->  /!\\ Client %s try to kick.\n",get_Pseudo(id_client)); 
							   sendMessage("You can't kick.",MSG_SERVER,id_client,NULL);
							   break;
					case  -2 : printf("*Serv_recep*    ->  /!\\ User not online  \n");
							   sendMessage("User not online.",MSG_SERVER,id_client,NULL);
							   break;
					case  -3 : printf("*Serv_recep*    ->  /!\\ Client %s try to kick %s \n",get_Pseudo(id_client),msg);
							   sendMessage("You can't kick this client",MSG_SERVER,id_client,NULL);
							   break;
					case  -4 : printf("*Serv_recep*    ->  /!\\ User not in channel  \n");
							   sendMessage("This user is not in this channel.",MSG_SERVER,id_client,NULL);
							   break;
					case  -5 : printf("*Serv_recep*    ->  /!\\ Client can't joined Public (kick)  \n");
							   sendMessage("There is to many clients in the Public channel.",MSG_SERVER,id_client,NULL);
							   break;
					default : printf("*Serv_recep*    ->  Client %s kick %s\n", get_Pseudo(id_client),get_Pseudo(error));
							  /*-1- Envoi message "client was kick by c" sur l'ancien channel du client + maj liste clients des autres clients du channel*/
					 		  sprintf(msg,"%s was kick by %s.",get_Pseudo(error),get_Pseudo(id_client));  
							  sendMessage(msg,MSG_SERVER,-1,get_Canal(id_client)); 
							  delListClient(msg,error,get_Canal(id_client));
							 
							  /*-2- Envoi message "You was kick by c" au client concerné*/
							  sprintf(msg,"You was kick by %s.",get_Pseudo(id_client));
							  sendMessage(msg,MSG_SERVER,error,NULL); 
							 
							  /*-3- Envoi message "client joined" sur le nouveau channel et lui envoie la liste des clients + maj liste clients des autres clients du channel*/
							  sprintf(msg,"%s has joined the channel.",get_Pseudo(error));  
							  sendMessage(msg,MSG_SERVER,-1,get_Canal(error)); 
							  sendListClient(msg,error);
							  break;
				}
				return;	
	
        		return;
		case WHISPER :
				error = parsePseudo(msg);
				switch (error) {
					case  -1 : printf("*Serv_recep*    ->  /!\\ Incorrect pseudo.\n"); 
							   sendMessage("Incorrect pseudo.",MSG_SERVER,id_client,NULL);
							   break;
					case  -2 : printf("*Serv_recep*    ->  /!\\ User not online  \n");
							   sendMessage("User not online.",MSG_SERVER,id_client,NULL);
							   break;
					default  : printf("*Serv_recep*    ->  Client %s send private message to %s\n", get_Pseudo(id_client),get_Pseudo(error));
								sendMessage(msg, id_client,error,NULL);													
								break;
				}
				return;
		case DISC :
				printf("*Serv_recep*    -> Client %s offline\n",get_Pseudo(id_client));
				/*-1- Envoi message "client has left c" le channel du client + maj liste clients des autres clients du channel*/
				sprintf(msg,"%s has left channel.",get_Pseudo(id_client));  
				sendMessage(msg,MSG_SERVER,-1,get_Canal(id_client));
				delListClient(msg,id_client,get_Canal(id_client));
				kill_Client(id_client);
        		return;
		case PING :
		        //augmente l'activité du client
				active_inc(id_client);
				return;
		default :
				sendMessage("Invalid command.",MSG_SERVER,id_client,NULL);
				return;  
    }
}

/* 
Cette fonction envoie sur le nouveau channel du client (id_client) que cette
personne à rejoint le channel, de plus elle envoie à ce nouveau client la liste
de tous les connectés sur ce channel.
*/
void sendListClient(char* msg ,int id_client) {  
	Liste_Client listeC = get_Liste(get_Canal(id_client));
	// envoie sur le channel que ce client est à ajouté à leur liste de client
	sprintf(msg,"%s%s",JOIN_LC,get_Pseudo(id_client));
	sendMessage(msg,MSG_SERVER,-1,get_Canal(id_client));
	// reset la liste du client
	sprintf(msg,"%s%s",RESET_LC,get_Name(get_Canal(id_client)));
	sendMessage(msg,MSG_SERVER,id_client,NULL);
	// envoie au client la liste de tous les connectés sur le channel	
	while(listeC!=NULL) {
    	sprintf(msg,"%s%s",JOIN_LC,get_Pseudo(listeC->id_client));
		sendMessage(msg,MSG_SERVER,id_client,NULL);
		listeC = listeC->suivant;
	}    
}

/* 
Cette fonction envoie sur l'ancien channel du client (id_client) que cette personne 
quitte le channel 
*/
void delListClient(char* msg ,int id_client, Liste_Channel canal) {
	// envoie sur le channel que ce client est à supprimer de leur liste de client
    sprintf(msg,"%s%s",LEAVE_LC,get_Pseudo(id_client));
	sendMessage(msg,MSG_SERVER,-1,canal);
}


/*
id_emetteur  = client E qui envoie le message si mit à -1 c'est un message serveur
id_recipient = client R qui doit recevoir le message 
broadcast  =  Channel sur lequel le message doit être envoyé à tous les clients
*/ 
void sendMessage(char* msg,int id_emetteur,int id_recipient,Liste_Channel broadcast) {
	
	char buff_write[SERVER_RECV_MSG];
	int fd_write;
    Liste_Client canal_lclients;
    
    // création du message
    strcpy(buff_write,"");
  
	if(id_emetteur!=MSG_SERVER) {
    	strcat(buff_write,get_Pseudo(id_emetteur));
        strcat(buff_write,":");
        if(id_recipient!=-1) 
		     strcat(buff_write,get_Pseudo(id_recipient));
	}
    strcat(buff_write,":");
    strcat(buff_write,msg);
    strcat(buff_write,"\n");
    	 
    // affichage du message
    printf("*Serv_recep*    >> send  %s",buff_write);
    
    // Sur tout le channel
    if(broadcast!=NULL) { 
    	canal_lclients = get_Liste(broadcast);   
		while(canal_lclients != NULL) {
			fd_write = open (get_Pipename(canal_lclients->id_client), O_RDWR);

			if ( fd_write == -1) {
				puts ("*Serv_recep*   /!\\ Can't open pipe");
				continue;
			}		

			if (( write(fd_write,buff_write, SERVER_RECV_MSG)) == -1) {
				puts ("*Serv_recep*  /!\\ Can't write into user pipe");
				close (fd_write);	
				continue; 
			}	
		
  			close (fd_write);	
  			canal_lclients = canal_lclients->suivant;
		}
	}
	else {
		// Sinon à une personne qui doit recevoir ce message
    	if(id_recipient!=-1) { 
			fd_write = open (get_Pipename(id_recipient), O_RDWR);

			if ( fd_write == -1) {
				puts ("*Serv_recep*   /!\\ Can't open pipe");
				return;
			}		
			if (( write(fd_write,buff_write, SERVER_RECV_MSG)) == -1) {
				puts ("*Serv_recep*  /!\\ Can't write into user pipe");
				close (fd_write);	
				return; 
			}		
  			close (fd_write);
		}

		// A la personne qui envoie le message (sauf si c'est le serveur)
		if(id_emetteur!=MSG_SERVER) {
			fd_write = open (get_Pipename(id_emetteur), O_RDWR);

			if ( fd_write == -1) {
				puts ("*Serv_recep*   /!\\ Can't open pipe");
				return;
			}		
			if (( write(fd_write,buff_write, SERVER_RECV_MSG)) == -1) {
				puts ("*Serv_recep*  /!\\ Can't write into user pipe");
				close (fd_write);	
				return; 
			}	
			close (fd_write);
		}
	}
}

int parseMessageSend(char * str,int* pid,long int* num,int* option,char * msg){
	int i=0, u=0,l=0,k=0,r=0;
	char pid_char[MAX_PIDLENGTH];
	char num_char[MAX_NUMGEN];
	char option_char[MAX_OPTION];
	
	while(str[i] != ':'){
		if(i == MAX_PIDLENGTH) return -1;
		pid_char[i] = str[i];
		i++;
	}
	pid_char[i] = '\0';
	*pid = atoi (pid_char);
	i++;
	while(str[i] != ':'){
		if(u == MAX_NUMGEN) return -1;
		num_char[u] = str[i];
		i++;u++;
	}	
	num_char[u] = '\0';
	*num = atol (num_char);
	i++;

	while(str[i] != ':'){
		if(l == MAX_OPTION) return -1;
		option_char[l] = str[i];
		i++;l++;
	}
	option_char[l] = '\0';
	*option = atoi (option_char);
	i++;

	while(str[i] != '\n'){
		if(k == MAX_MSG_LENGTH) return -1;
		msg[k] = str[i];
		i++;k++;
	}
	msg[k] = '\0';
	return 0;
}
