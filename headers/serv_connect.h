#include "config_server.h"
#include "../tools/SFhash/SFhash.h"


/** 
 * En cas d'erreur ou fin de programme serv_recep
 */
void exitConnet(int status);


/**
 * The connexion reader should always be occuped by a process. When a client
 * is writing a log sentence, the CR will verify into the log file to 
 * authentificate.
 */
void connexionReader();


/**
 * Will analyse readed buffer at connexion.
 * Add the client into plugged user list.
 * Send message back to client.
 */
void connexionProcess(char *line);

/** <p>Découpe un message de connexion donné "str" et la parse en retournant
 *  un login, un password, un entier pid et une chaine pipe_num
 *  Si un des arguments manque dans str ou qui n'est pas au bon format
 * (ex login de 20 caractères), la fonction retourne -1</p>
 **/
int parseConnexionAsk(char * str, char * login, char * password,int* pid, char * pipe_num);


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
int isAllowedUser(char* login, int32_t mdpint,char* pseudo);

