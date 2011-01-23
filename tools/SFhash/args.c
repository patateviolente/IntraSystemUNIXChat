#include <stdio.h>
#include <stdlib.h>
#include <string.h>



void decryptArgs(int argc, char ** argv){
	int eq, len, i;
	char nom[MAXARGNAME_LEN], value[MAXARGVALUE_LEN];
	char c;
	char * rec;
	char tmp[50];
	
	
	for(i=1; i<argc; i++){
		
		// 
		rec = strchr(argv[i], atoi("="));
		len = strlen(argv[i]);
		eq = len - strlen(rec) - 1;
		if(eq > MAXARGNAME_LEN || (len-eq) > MAXARGVALUE_LEN){ puts("err"); continue;}
		else{
			// ajout liste
			strncpy(tmp, argv[i], eq);
			strcpy(tmp, argv[i]);
			printf("Argument %s vaut %s", tmp, tmp);
		}
	}
}
