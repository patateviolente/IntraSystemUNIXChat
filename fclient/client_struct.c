#include "../headers/client_struct.h"


int cutMessage(char line[]){
	char c;
	int i=0, u=0;
	
	// capture pseudo
	while (line[i] != ':' && line[i] != '\0')
		msgcut->pseudo[u++] = line[i++];
	msgcut->pseudo[u] = '\0';
	
	if(line[++i] != ':' && msgcut->pseudo[0] != '\0'){		// WHISPER : pseudode:pseudoa:message
		u = 0;
		char pseudode[MAX_PSEUDO_LENGTH];
		char msgtmp[MAX_MSG_LENGTH+MAX_PSEUDO_LENGTH+10];
		// capture pseudo from 
		while (line[i] != ':' && line[i] != '\0')
			pseudode[u++] = line[i++];
		pseudode[u] = '\0';
		
		//printf("PSEUDO='%s'   ++   pseudoDE='%s'\n", pseudo, pseudode);
		
		msgcut->msg[0] = '\0';		// erase previous msg	
		strcat(msgcut->msg, (strcmp(pseudo, pseudode) == 0) ? "[FROM " : "[TO ");
		strcat(msgcut->msg, (strcmp(pseudo, pseudode) == 0) ? msgcut->pseudo : pseudode );
		strcat(msgcut->msg, "]");
		u = strlen(msgcut->msg);
	} else 
		u = 0;
	
	
	// capture msg
	if(msgcut->pseudo[0] != '\0') i++;		// not server msg
	while (line[i] != '\0' && line[i] != '\n')
		msgcut->msg[u++] = line[i++];
	msgcut->msg[u] = '\0';
	
	return 0;
}


int addClient(char line[]){	
	int i=0, u, len = strlen(line);
	char pseudo[MAX_PSEUDO_LENGTH+1];
	if((len+3) > MAX_PSEUDO_LENGTH) return -1;					// too long
	
	// get pseudo from line type ::#+pseudo
	for(u=3; u<len-1; u++) pseudo[i++] = line[u];
	pseudo[i] = '\0';
	
	/* useless ?
	// already exists ?
	for(u=0; u<users->nb; u++)
		if(strcmp(users->pseudo[u], pseudo) == 0) return 1;		// already exists
	if(users->nb >= MAX_USR_STORED) return 2;					// no more space
	*/
	
	// add into array
	strcpy(users->pseudo[users->nb], pseudo);
	users->nb++;
	
	return 0;
}

int removeClient(char line[]){
	int i=0, u, len = strlen(line);
	char pseudo[MAX_PSEUDO_LENGTH+1];
	if((len+3) > MAX_PSEUDO_LENGTH) return -1;					// too long
	
	// get pseudo from line type ::#+pseudo
	for(u=3; u<len-1; u++) pseudo[i++] = line[u];
	pseudo[i] = '\0';
	
	// Search user
	for(u=0; u<users->nb; u++){
		if(strcmp(users->pseudo[u], pseudo) == 0){
			// remove him
			for(; u<users->nb-1; u++)
				strcpy(users->pseudo[u], users->pseudo[u+1]);
			users->nb--;
			return 0;
		} //else printf(" '%s' != '%s'\n", pseudo, users->pseudo[u]);
	}
	
	/* puts("\n## DISP :");
	for(u=0; u<users->nb; u++)
		printf("%s\n", users->pseudo[u]); */
	
	return 1;													// does not exists
}

void removeAllClient(){
	users->nb = 0;
}
