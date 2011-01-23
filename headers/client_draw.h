#include "client_chat.h"
#include <ncurses.h>
#include <curses.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#define POPUPWIN_WIDTH 60		// popup width (chars)
#define POPUPWIN_HEIGHT 6		// popup height (lines)


/**
 *  * At opening */
//void initInterface();

/**
 * Quit chat, end curses properly */
void endInterface();

/** 
 * Fonction to call when window has been resizd : repaint all graphics  */
void onResize();

/**
 * Repaint ALL : windows, chat, users  */
int repaint();



/** 
 * @arg txt chaîne de caractères
 * @return nombre de lignes nécessaires pour afficher le texte en argument dans
 * 	la fenêtre de chat
 */
int getNbLines(char txt[]);

/** 
 * @return Rang de la première ligne à afficher pour que toute la fin de la 
 * discution soit visible 
 */
int getFirstLineOffset();

/** Open new window at the center of the screen
 * @txt Message displayed before read
 * @len mximum chars (\0 included) 
 */
void popupGetText(const char txt[], const int len, int opt);

/**
 * Display input on bottom.
 * Never stop.
 */
void getText();



/****************************************************************************/
struct sigaction sigResize;							// window resize signal
struct sigaction sigexit;							// window resize signal

WINDOW *wchato, *wchat, 	*wconnecto, *wconnect;	// curses window containing chat/ users content
int chatCols, chatRows, dispUsers = 1;				// graphics vars (w*h)
enum { C_BG, C_CHAT, C_CHAT_B, C_PSEUDO, C_CNCT, C_CNCT_O, C_TOPINFOS  };		// choose COLORS

char str[MAX_MSG_LENGTH];							// input str
char rep[50];										// str for popup (MAX)
enum {STD, QUIT, HELP};		// precise popup window type for popupGetText func

/****************************************************************************/
