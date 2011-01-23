/****************************************************************************/
#include <stdio.h>			// fflush, getchar, fgetc
#include <string.h>			// strchr
#include <stdlib.h>
#include <termios.h>		// pass
#include <string.h>
#include "../headers/config_client.h"

/**
 * Clean standard input from keyboard.
 * If a too big string is typed and fgets refused to take them all. Buffer
 * should be clear
 */
void clearBuffer();

/**
 * Securized scan : put in str the standard keyboard input but with a maximum
 * length caracters. Then throw the buffer.
 * @param str Char array where text must be placed
 * @param length Maximum caracters to put in array (not counting \0 !)
 * @return 
 *    <ul>
 *       <li>0 if ok</li>
 *       <li>-1 if input truncated (and cleared)</li>
 *       <li>1 if fgets failed</li>
 *    </ul>
 */
int readKeyboard(char *str, const int length);


/**
 * Thanks to this answer to use another function than getpass (obsolete)
 * http://stackoverflow.com/questions/1196418/getting-a-password-in-c-without-using-getpass-3
 * Disable echo, and capture keyboard with fgets
 */
void getThisPass();


/** 
 * @param arg0 First arg (ex ./bin/serverchat) - not const
 * @return Relative path (ex ./bin/)
 */
char * getProgRelativePath(char * arg0);

/****************************************************************************/
