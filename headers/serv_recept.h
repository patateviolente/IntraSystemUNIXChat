#include "config_server.h"


/** 
 * En cas d'erreur ou fin de programme serv_recep
 */
void exitRecep(int status);

/** Read continuously reception pipe, verify the member authorisation. 
 * Display it to the chanel he is registered.
 */
void listenMsgsPipe();

/** <p> Analyse un message d'un client et suivant l'option choisit la traite et 
 * répond au(x) client(x) </p>
 */
void messageProcess(char * line);


/** <p>Découpe un message de connexion donné "str" et la parse en retournant
 *  un pid, un num, un entier option et une chaine message
 *  Si un des arguments manque dans str ou qui n'est pas au bon format
 * (ex pid de 20 caractères), la fonction retourne -1</p>
 **/
int parseMessageSend(char * str,int* pid,long int* num,int* option,char * msg);


/** <p> Cette fonction envoie un message msg, elle crée le message et l'envoie au(x) destinataire(s)
 * @param id_emetteur  = client E qui envoie le message si mit à -1 c'est un message serveur
 * @param id_recipient = client R qui doit recevoir le message 
 * @param broadcast  =  Channel sur lequel le message doit être envoyé à tous les clients</p>
 **/
void sendMessage(char* msg,int id_emetteur,int id_recipient,Liste_Channel broadcast);

/** 
 *  <p>Cette fonction envoie sur le nouveau channel du client (id_client) que cette
 *  personne à rejoint le channel, de plus elle envoie à ce nouveau client la liste
 *  de tous les connectés sur ce channel. </p>
 **/
void sendListClient(char* msg ,int id_client);

/** 
 *  <p>Cette fonction envoie sur l'ancien channel du client (id_client) que cette personne 
 *  quitte le channel</p>
 **/
void delListClient(char* msg ,int id_client, Liste_Channel old_canal);
