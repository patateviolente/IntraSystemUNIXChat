#include "SFhash.h"
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>

/**
 * Hash password test  
 */
int main(int argc, char ** argv){
	if(argc > 1)		// direct
		printf("%d\n", SuperFastHash(argv[1], strlen(argv[1])));
	else{				// interractive
		char str[500] = "no_pass";
		puts("### PASSWORD HASHER ###");
		printf("Enter you password (unhidden) : ");
		scanf("%s", str);
		printf("The password '%s' => '%d'\n", str, SuperFastHash(str, strlen(str)));
	}
	return 0;
}
