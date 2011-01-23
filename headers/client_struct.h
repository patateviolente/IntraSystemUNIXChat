#include "config_client.h"

/** put into msgcut structure chanel name, pseudo & msg **/
int cutMessage(char line[]);

/** add client into users structure from server info line (::#+ type)
 * @return :
 * 0 if added
 * 1 if already exists
 * 2 array too short (see MAX_USR_STORED)
 * -1 pseudo too long (see MAX_PSEUDO_LENGTH)
 */
int addClient(char line[]);

/** remove client from users structure from server info line (::#- type)
 * @return :
 * 0 if removed
 * 1 if does not exists
 * -1 pseudo too long (see MAX_PSEUDO_LENGTH)
 */
int removeClient(char line[]);

/**
 * Remove all clients, just move cursor on first offset **/
void removeAllClient();
