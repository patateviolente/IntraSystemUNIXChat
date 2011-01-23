#include "headers/io_extend.h"			// readKeyboard

/** SEE BELLOW */
void decryptArgs(int argc, char ** argv);
struct sigaction sigexit;							// window resize signal


/**
 * Welcome to client :
 *  - Call connexion procedure (pipe created) then
 *  - Read 
 */
int main(int argc, char **argv){
	char rep[4];		// try again ?
	int connexStatus;	// is connexion enabled ?
	decryptArgs(argc, argv);
	
	argv[0][strlen(argv[0])-6] = '\0';		// remove "client$" (get ./ or any else)
   	chdir(argv[0]);
   	
	// go pipe folder
	if(chdir(PIPE_FOLDER) == -1){ 
		perror("Can't access pipe folder");
		return 7;
	}
	
	// Generate random number (for pipe name) into pipe_number (see config header)
	srand(getpid());
	pipe_number = (long int)(rand()/(float)RAND_MAX * (pow(10,(MAX_NUMPIPE-1)))); 
	sprintf(pipe_num, "%ld", pipe_number);
	
	// CONNEXION...
	connexion:
	puts("## Chat connexion assistant : log > chat");
	
	connexStatus = connexion();
	if(connexStatus == 0){
		// Then chat
		puts("\n## Connexion is now etablished, starting chat ... ");
	} else{
		// Should try again...
		printf("<< Want to try again ([oy|enter] / n) ? ");
		readKeyboard(rep, 4);
		if(rep[0] == 'y' || rep[0] == 'o' || rep[0] == '\0')
			goto connexion;
		return 0;
	}
	
	// CHAT
	clientChat();
	return 0;
}




/**
 * Apply args. Warning : can stop program.
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
	// = 0;
	o_gui = o_guicolors = o_colors = 1;
	
	for(i=1; i<argc; i++){		// all args
		len = strlen(argv[i]);
		// option type -x
		if(len == 2 && argv[i][0] == '-'){
			switch(argv[i][1]){
				case 'g': o_gui = 0; break;						// Console
				case 'c': o_colors = !o_colors; break;			// console Colors
				case 'G': o_gui = 1; break;						// GUI
				case 'C': o_guicolors = !o_guicolors; break;	// GUI : colors
			}
		}
	}
}


