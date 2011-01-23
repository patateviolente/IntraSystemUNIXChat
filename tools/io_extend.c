#include "../headers/io_extend.h"
/** infos in io_extend.h **/

void clearBuffer(){
    int c = 0;
    while (c != '\n' && c != EOF)
        c = getchar();
}

int readKeyboard(char *str, const int length)
{
    char *brPosition = NULL;
 
    // scaning entry
    if (fgets(str, length+1, stdin) != NULL) {
        brPosition = strchr(str, '\n'); // 
        if (brPosition != NULL) // backrow detected
            *brPosition = '\0'; 	// replacing backrow
        else{
			clearBuffer();
			return -1;
        }
        return 0;
    }
    else{
		clearBuffer();
		return 1; // On renvoie 0 s'il y a eu une erreur
	}
}

void getThisPass(char * password){
	struct termios oflags, nflags;

    /* disabling echo */
    tcgetattr(fileno(stdin), &oflags);
    nflags = oflags;
    nflags.c_lflag &= ~ECHO;
    nflags.c_lflag |= ECHONL;
	
    if (tcsetattr(fileno(stdin), TCSANOW, &nflags) != 0) {
        perror("tcsetattr");
    }
	
    fgets(password, sizeof(password), stdin);
    password[strlen(password) - 1] = 0;
    // printf("you typed '%s'\n", password);

    /* restore terminal */
    if (tcsetattr(fileno(stdin), TCSANOW, &oflags) != 0) {
        perror("tcsetattr");
    }
}

char * getProgRelativePath(char * arg0){
	int len = strlen(arg0);
	while(len > 0 && arg0[len] != '/') len--;
	arg0[len+1] = '\0';
	return arg0;
}
