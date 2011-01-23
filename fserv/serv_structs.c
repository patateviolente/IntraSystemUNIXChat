#include "../headers/serv_structs.h"
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>


/* ------------------------------ INITIALISATION ------------------------ */

/** Création des variables partagés et du mutex (le tableau de client)
   0 = réussi
  -1 = échec (shmalloc)(mutalloc) **/
int init_Client() {
	int i;
	// création de la sémaphore pour le tableau de client
	key_t key = ftok("/etc/magic", 0);
	
	if (( mut = mutalloc(key)) == 0) {
		return (-1); }
	
	// allocation du tableau de client (mémoire partagé)
	key =ftok("/etc/passwd",1);
	if (( clients = (Client*)shmalloc(key,(sizeof(Client)*MAX_USER)) ) == 0) {
		exit_Client();return (-1); }
	
	for(i=0;i<MAX_USER;i++) // aucun client
		clients[i].type=0;
		
	// allocation et initialisation de l'id du prochain client (mémoire partagé)
	key =ftok("/etc/passwd",2);
	if (( new_id_client = (int*)shmalloc(key,sizeof(int)) ) == 0) {
		exit_Client();return (-1); }
	
	*new_id_client=0; // id du premier client à créer
	
	return(0);
}
		
/** Création du premier channel (acceuil)
   0 = réussi
  -1 = échec (malloc) */
int init_Channel() {
	if(( channels = (Liste_Channel)malloc(sizeof(*channels)) )==0) return (-1);      
	channels->id_createur_channel = -1; // ne doit pas avoir de créateur pour éviter les kick sur ce channel !
	channels->client_channel = NULL;  // aucun client
	strcpy(channels->nom_channel,"Public");
	channels->suivant = NULL;
	return (0);
}


/* ---------------- FONCTIONS SUR LES CLIENTS ET CHANNELS --------------- */


/** Insère un nouveau client 
   0 = réussi
  -1 = échec (trop de client) */
int new_Client_connect(char *pipe_name,char *pseudo,int pid,long int num,short int type) {
	if(*new_id_client<MAX_USER) {
	    P(mut); /*-- zone critique --*/
	    clients[*new_id_client].type=type;
	    clients[*new_id_client].active=2; 
		strcpy(clients[*new_id_client].pipe_name, pipe_name);  
		strcpy(clients[*new_id_client].pseudo, pseudo); 
		clients[*new_id_client].pid=pid;
		clients[*new_id_client].num=num;
		kill(recep_pid,15); // envoi à serv_recep un kill pour appelé la fonction new_Client_recept()
		// car serv_conne ne connait pas les channels et ne peut donc pas l'initialisé
		return (0);
	}
 	// sinon 
	return(-1);     
}

/** Initialise le canal du nouveau client 
   0 = réussi
  -1 = échec (trop de client dans le channel) */
int new_Client_recept() {
	int new_id;  
	int old_id=*new_id_client;
	// ajoute le client au premier channel de la liste channels (soit le channel Public)
    if((add_Client(&channels->client_channel,*new_id_client)) == 0) {
    	// link le client à ce channel
        clients[*new_id_client].canal = channels;
        // recherche le prochain client non connecté soit le prochaine id libre
        new_id=*new_id_client+1;
        while(clients[new_id].type!=0 && new_id<MAX_USER)
			new_id++;  	
        *new_id_client=new_id;
        V(mut);	/*-- fin zone critique --*/
		return(old_id);
	}	
	//sinon
    V(mut);	/*-- fin zone critique (même si échec)--*/
    return(-1);
}


/** Supprime un client
   0 = réussi
  -1 = échec (client introuvable) */
int kill_Client(int id_client) {
	if(id_client>=0 && id_client<MAX_USER && clients[id_client].type!=0) {
	    P(mut); /*-- zone critique --*/
	    unlink(clients[id_client].pipe_name);
	    del_Client(&clients[id_client].canal->client_channel,id_client);
	    // si le channel qu'il à quitté est vide, ce channel est détruit (sauf si c'est le channel Public")
		if(clients[id_client].canal->client_channel==NULL && (strcmp(clients[id_client].canal->nom_channel,"Public")!=0) )
				kill_Channel(clients[id_client].canal);
		// si ce client était le responsable du channel, un nouveau responsable est attribué (le plus ancien client du channel)
		else if(clients[id_client].canal->id_createur_channel==id_client) 
			getNewCreator(clients[id_client].canal);
	    // met son type à 0 ; donc ce client est déconnecté
	    clients[id_client].type=0;
	    // met à jour le prochaine id libre pour l'insertion d'un client
	    if(id_client<*new_id_client)
	    	*new_id_client=id_client;
	    V(mut);	/*-- fin zone critique --*/
		return (0);
	}
	//sinon
	return (-1);	  	
}

 

/** Trouve un nouveau responsable du channel par défault le dernier 
de la liste càd le plus ancien connecté sur ce channel */
void getNewCreator(Liste_Channel  channel) {
    Liste_Client  liste=channel->client_channel;	
	while(liste->suivant!= NULL) {
		liste=liste->suivant;
	}
	channel->id_createur_channel=liste->id_client;
}

/** Allocation d'un nouveau channel
   0 = réussi
  -1 = échec (channel déja existant)
  -2 = échec (malloc)
  -3 = échec (add client) */
int new_Channel(char *nom_channel, int id_createur_channel) {
	Liste_Channel new_channel= channels;
	// si le channel existe déja retourne -1
	while(new_channel!=NULL) {
		if((strcmp(new_channel->nom_channel,nom_channel))==0)
			return(-1);
		new_channel=new_channel->suivant;
	}
	// si l'allocation échoue retourne -2
	if((new_channel=(Liste_Channel)malloc(sizeof(*new_channel)))   ==   0) return (-2);
	
	new_channel->id_createur_channel = id_createur_channel ; 
	new_channel->client_channel = NULL ;  	// aucun client
	strcpy(new_channel->nom_channel,nom_channel);
	
	// si l'ajout du créateur dans ce channel échoue retourne -3
	if( add_Client(&new_channel->client_channel,id_createur_channel) ==-1)
		{ free(new_channel); return (-3); }		 // ajout du créateur
    
    // supprime le créateur  de son ancien channel
    del_Client(&clients[id_createur_channel].canal->client_channel,id_createur_channel); 
    
     // si le channel qu'il à quitté est vide, ce channel est détruit (sauf si c'est le channel Public")
	if(clients[id_createur_channel].canal->client_channel==NULL && (strcmp(clients[id_createur_channel].canal->nom_channel,"Public")!=0) )
			kill_Channel(clients[id_createur_channel].canal);
	// sinon si ce client était le responsable du channel, un nouveau responsable est attribué (le plus ancien client du channel)
	else if(clients[id_createur_channel].canal->id_createur_channel==id_createur_channel) 
			getNewCreator(clients[id_createur_channel].canal);
	
	// ajout du channel à la liste des channels
    clients[id_createur_channel].canal=new_channel; // link  le client au nouveau channel
	new_channel->suivant=channels->suivant;        // ASTUCE ->suivant pour garder le canal Public en premier dans la liste
	channels->suivant=new_channel;                // Link le channel à la liste des channel  
	return (0);
}


/** Rejoindre un channel
   0 = réussi
  -1 = échec (channel introuvable)
  -2 = échec (déjà dans ce channel)
  -3 = échec (add client) */
int join_Channel(char *nom_channel, int id_client) {
	Liste_Channel channel = channels;
	Liste_Channel old_channel = clients[id_client].canal;
	//recherche du channel
	while(channel!=NULL) {
		if((strcmp(channel->nom_channel,nom_channel))==0)
			break;
		channel=channel->suivant;
	}
	// si le channel n'existe pas retour -1
	if(channel==NULL) 
		return(-1);
	// si le client  est déja dans ce channel pas retour -2
	if(clients[id_client].canal==channel) 
		return(-2);
	// si l'ajout du client dans ce channel échoue retourne -3
	if( add_Client(&channel->client_channel,id_client) ==-1)
		return(-3);
	
	 // supprime le client de son ancien channel
	del_Client(&old_channel->client_channel,id_client);
	
	// si le channel qu'il à quitté est vide, ce channel est détruit (sauf si c'est le channel Public")
	if(old_channel->client_channel==NULL && (strcmp(old_channel->nom_channel,"Public")!=0) )
			kill_Channel(old_channel);
	// sinon si ce client était le responsable du channel, un nouveau responsable est attribué (le plus ancien client du channel)
	else if(old_channel->id_createur_channel==id_client) 
			getNewCreator(old_channel);
	
	// link  le client au nouveau channel	
    clients[id_client].canal=channel;
	
    return(0);
}

/** Suppresion d'un channel suppose que ce channel existe*/
int kill_Channel(Liste_Channel del_channel) {
	Liste_Channel* curseur=&channels; // pointe sur le channel à comparé
	Liste_Channel* curseur_pred=NULL; // pointe sur son prédécesseur
	//recherche le channel
	while(del_channel != ((Liste_Channel)(*curseur))) {
	    curseur_pred=curseur;
		curseur=&((Liste_Channel)(*curseur))->suivant;
	}
	// Fait pointé son prédécesseur sur son suivant
	((Liste_Channel)(*curseur_pred))->suivant  = del_channel->suivant ;
	// Libère l'allocation du channel
	free(*curseur); 
}

/** Kick d'un client, msg étant le pseudo du clientS
   0> = réussi et retourne l'id du client kick
  -1 = échec (aucun droit de kick) 
  -2 = échec (personne à kick introuvable) 
  -3 = échec (impossible de kick cette personne) 
  -4 = échec (impossible de kick dans ce channel) 
  -5 = échec (ajout du client dans Public échec) */
int kick(char *msg,int id_client) {
	int id_kick;
	// si le client/admin n'est pas responsable de son channel et si il n'est pas admin retour -1
	if((clients[id_client].canal->id_createur_channel != id_client) && (clients[id_client].type!=2)  )
		return(-1);
	// récupère l'id
	id_kick = verifPseudo(msg);
	// si le pseudo ne correspond à aucun client retour -2
	if(id_kick==-1) 
		return(-2);
	// si le client/admin essaye de se kick lui_même ou de kick un admin retour -3
    if(id_kick==id_client || clients[id_kick].type==2)
		return(-3);
	// si le client/admin essaye de kick un client qui n'est pas dans son channel et que le channel est Public retour -4
	if((clients[id_kick].canal!=clients[id_client].canal) && (clients[id_kick].canal!=channels) )
		return(-4);
	// si l'ajout du client kick dans Public échoue retour -5
	if(join_Channel("Public",id_kick)!=0)
		return(-5);
	return(id_kick);
}

/** Ajout d'un client dans une liste (liste des clients de channel)
   0 = réussi
  -1 = échec (malloc) */
int add_Client(Liste_Client *liste,int id_client) { 
	Liste_Client lc;
	// crée le client
	if(( lc=(Liste_Client)malloc(sizeof(*lc)) )  ==  0) return (-1); 
	// l'ajoute en tête de liste O(1) insertion rapide   
	lc->suivant=*liste;
	lc->id_client=id_client;
	*liste = lc; 
	return (0);   
}


/** Suppresion d'un client dans une liste (liste des clients du channel) 
	Suppose que le client existe dans la liste
*/
void del_Client(Liste_Client* liste,int id_client) {
	Liste_Client* curseur=liste; // pointe sur le client à comparé
	Liste_Client* curseur_pred=NULL; // pointe sur son prédécesseur
    Liste_Client  curseur_suiv; // pointe sur son successeur
	//recherche le client
	while(id_client != ((Liste_Client)(*curseur))->id_client) {
	    curseur_pred=curseur;
		curseur=&((Liste_Client)(*curseur))->suivant;
	}
    curseur_suiv = ((Liste_Client)(*curseur))->suivant;
    
    // Libère l'allocation du client dans la liste
	free(*curseur); 

	// si il a un prédécesseur ( pas le premier de la liste)
	if(curseur_pred != NULL) 
		// Fait pointé son prédécesseur sur son suivant
    	((Liste_Client)(*curseur_pred))->suivant = curseur_suiv;
	else 
		// Fait pointé la liste (premier élement) sur son suivant
	    *liste = curseur_suiv;
}


/* --------------------- FONCTIONS DE VERIFICATION ---------------------- */

/** Fonction spécial, récupère l'id d'un client à partir d'un format de message spécial,
	celui des messages whisper: 
	exemple : pseudo#texte ou je dis des choses secrètes
	Cette fonction doit récupérer le pseudo et renvoyer l'id correspondant tout en effacant
	du message le 'pseudo#'
   >0 réussi id_client
  -1 = échec (client introuvable)
  -2 = échec (format du message incorrect)*/
int parsePseudo(char * msg) {

	char pseudo[MAX_PSEUDO_LENGTH];
	int i=0,k=0;
	// récupère le  pseudo
	while(msg[i] != '#'){
		if(i == MAX_PSEUDO_LENGTH) return (-2);
		pseudo[i] = msg[i];
		i++;
	}
	pseudo[i] = '\0';
	i++;
	// supprime le 'pseudo#' du messahe
	while(msg[i] != '\0'){
		msg[k] = msg[i];
		i++;k++;
	}
	msg[k] = '\0';

	// retourne l'id du pseudo si il existe
	return(verifPseudo(pseudo));
}

/** Récupère l'id d'un client à partir de son pseudo
   >0 réussi id_client
  -1 = échec */
int verifPseudo(char *pseudo) {
    int i=0;
	while(i < MAX_USER) {
	 	if(clients[i].type != 0)
			if(strcmp(clients[i].pseudo,pseudo)==0) break; 
		i++;
	}
	if(i==MAX_USER)
		return (-1);	
    return(i);
}


/** Fonction spécial pour les ping ; décrémente l'activité du client
*/
void active_dec(int id_client){
	clients[id_client].active--;
}

/** Fonction spécial pour les ping ; incrémente l'activité du client
*/
void active_inc(int id_client){
	clients[id_client].active++;
}

/* -------------------- FONCTIONS DE GET ----------------------- */


/** Récupère l'id d'un client à partir de son pid et num
   >0 réussi id_client
  -1 = échec */
int get_Id_Client(int pid,long int num) {
	int i=0;
	while(i < MAX_USER) {
	 	if(clients[i].type != 0)
			if(clients[i].num == num  &&  clients[i].pid == pid) return i; 
		i++;
	}
	return (-1);
}

int get_Pid(int id_client) {
	return (clients[id_client].pid);
}

long int get_Num(int id_client) {
	return (clients[id_client].num);
}

int get_Type(int id_client) {
	return (clients[id_client].type);
}

int get_Active(int id_client) {
	return (clients[id_client].active);
}

char* get_Pseudo(int id_client) {
	return (clients[id_client].pseudo);
}

char* get_Pipename(int id_client){
	return (clients[id_client].pipe_name);
}

Liste_Channel get_Canal(int id_client){
	return (clients[id_client].canal);
}

Liste_Client get_Liste(Liste_Channel channel) {
	return (channel->client_channel);
}

char* get_Name(Liste_Channel channel) {
	return (channel->nom_channel);
}

/* ------------------------------- SUPPRESSION ------------------------------ */

void exit_Client() {
	int i;	
	// supprime les pipes
	for(i=0;i<MAX_USER;i++) {
		if(clients[i].type==1)
			unlink(clients[i].pipe_name);	
	}
	// libère les mémoires partagés 
    mutfree(mut);    
    key_t key = ftok("/etc/passwd", 1);
    shmfree(key);
    key = ftok("/etc/passwd", 2);
    shmfree(key);
}

void exit_Channel() {
	// libère le malloc de channels (dans serv_recep)
	free(channels);
}


