#include "../headers/config_client.h"   // Variables defined during connexion procedure



/**Interface of chat, call : 
 * <ul>
 * 		<li>listenChat : Display answers hearding `pipe_name`.</li>
 * 		<li>display connected users hearding CUSERS_PIPE</li>
 * 		<li>Can send answers, juste type</li>
 * </ul> */
void clientChat();

/** <p>Listen `pipe_name` for text and display it.
 * Stop only when pipe destroyed, return nothing.</p> **/
int listenChat();

/** Capture message and send it to server. */
int promptField();

/** Send message on reception pipe, decrypt argument **/
int sendMessage(int option,char buff[]);

/** A la réception d'un message, demande une mise à jour de l'ihm **/
void callRepaint();


