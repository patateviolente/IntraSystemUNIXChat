#include "../headers/ipctools.h"
#include "config.h"
// serv_struct is included by config_server.h

#define MAX_USER				10000	// Maximum users 

typedef struct pClient{
	int id_client;
	struct pClient *suivant;
} *Liste_Client;

typedef struct Channel{
	Liste_Client client_channel;
	char  nom_channel[MAX_CHANNEL_LENGTH];
	int id_createur_channel;
	struct Channel *suivant;
} *Liste_Channel;

typedef struct {
	short int type;    // 0 = libre / 1 = client / 2 = admin
	short int active;  // requête ping 
    char pipe_name[MAX_PIPE_NAME];
	char pseudo[MAX_PSEUDO_LENGTH];
    int pid; 
	long int num; 		// généré par le serveur à l'authentification (sécurité)
	Liste_Channel canal;
} Client;

// variables privées
static int* new_id_client; 		// id du prochain client (pour la connexion)
static int mut;					// mutex sur la variable partagé clients
static Client* clients;			// Liste des clients (serv_conne / serv_recep)
static Liste_Channel channels;	// Liste des channels (serv_recep)

extern int recep_pid; // donnée par smain (server.c)



/* ----- INITIALISATION ----- */

/** Création des variables partagés et du mutex (le tableau de client)
   0 = réussi
  -1 = échec (shmalloc)(mutalloc) **/
int init_Client();

/** Création du premier channel (acceuil)
   0 = réussi
  -1 = échec (malloc) */
int init_Channel();


/* ----- FONCTIONS SUR LES CLIENTS ET CHANNELS ----- */

/** Insère un nouveau client 
   0 = réussi
  -1 = échec (trop de client) */
int 	new_Client_connect(char *pipe_name,char *pseudo,int pid,long int num,short int type);

/** Initialise le canal du nouveau client 
   0 = réussi
  -1 = échec (trop de client dans le channel) */ 
int 	new_Client_recept();
  
/** Supprime un client
   0 = réussi
  -1 = échec (client introuvable) */
int 	kill_Client(int id_client);

/** Trouve un nouveau responsable du channel par défault le dernier 
de la liste càd le plus ancien connecté sur ce channel */
void 	getNewCreator(Liste_Channel  channel);

/** Allocation d'un nouveau channel
   0 = réussi
  -1 = échec (channel déja existant)
  -2 = échec (malloc)
  -3 = échec (add client) */
int 	new_Channel(char *nom_channel,int id_createur_channel);

/** Rejoindre un channel
   0 = réussi
  -1 = échec (channel introuvable)
  -2 = échec (déjà dans ce channel)
  -3 = échec (add client) */
int 	join_Channel(char *nom_channel, int id_client);

/** Suppresion d'un channel suppose que ce channel existe*/
int 	kill_Channel(Liste_Channel del_channel);

/** Kick d'un client, msg étant le pseudo du clientS
   0> = réussi et retourne l'id du client kick
  -1 = échec (aucun droit de kick) 
  -2 = échec (personne à kick introuvable) 
  -3 = échec (impossible de kick cette personne) 
  -4 = échec (impossible de kick dans ce channel) 
  -5 = échec (ajout du client dans Public échec) */
int 	kick(char *msg,int id_client);

/** Ajout d'un client dans une liste (liste des clients de channel)
   0 = réussi
  -1 = échec (malloc) */
int 	add_Client(Liste_Client *liste,int id_client);

/** Suppresion d'un client dans une liste (liste des clients du channel) 
	Suppose que le client existe dans la liste
*/
void 	del_Client(Liste_Client *liste,int id_client);


/* ----- FONCTIONS DE VERIFICATION ----- */

/** Fonction spécial, récupère l'id d'un client à partir d'un format de message spécial,
	celui des messages whisper: 
	exemple : pseudo#texte ou je dis des choses secrètes
	Cette fonction doit récupérer le pseudo et renvoyer l'id correspondant tout en effacant
	du message le 'pseudo#'
   >0 réussi id_client
  -1 = échec (client introuvable)
  -2 = échec (format du message incorrect)*/
int 	parsePseudo(char * msg);

/** Récupère l'id d'un client à partir de son pseudo
   >0 réussi id_client
  -1 = échec */
int 	verifPseudo(char *pseudo);

/** Fonction spécial pour les ping ; décrémente l'activité du client*/
void 	active_dec(int id_client);

/** Fonction spécial pour les ping ; incrémente l'activité du client*/
void 	active_inc(int id_client);


/* ----- FONCTIONS DE GET ----- */

/** Récupère l'id d'un client à partir de son pid et num
   >0 = réussi id_client
  -1 = échec */
int 			get_Id_Client(int pid,long int num);

int 			get_Pid(int id_client);
long int 		get_Num(int id_client);
int 			get_Type(int id_client) ;
int 			get_Active(int id_client);
char* 			get_Pseudo(int id_client);
char* 			get_Pipename(int id_client);
Liste_Channel 	get_Canal(int id_client);
Liste_Client 	get_Liste(Liste_Channel channel);
char* 			get_Name(Liste_Channel channel);


/* ----- SUPPRESSION ----- */

void exit_Client();
void exit_Channel();


