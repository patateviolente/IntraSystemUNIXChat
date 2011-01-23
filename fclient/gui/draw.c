#include "../../headers/client_draw.h"

int scr = 0;

void onResize(){
	endwin();
	refresh();
	repaint();
}

void endInterface(){
	popupGetText("Exit chat ? [yoq /n] : ", 3, QUIT); 		// ask
	if(!(rep[0] == 'y' || rep[0] == 'o' || rep[0] == '\n' || rep[0] == 'q')) return;
	
	endwin();		// end curses mode
	echo();
	exitChat();		// 
}


void initInterface(){
	initscr();	// init curses mode (fullscreen)
	
		// repaint signal
	sig_newmsg.sa_handler = callRepaint;
	sigaction(NEW_MESSAGE, &sig_newmsg, 0); 
	
	// resize signal
	sigResize.sa_handler = &onResize;
	sigaction(SIGWINCH, &sigResize, 0);
	
	// exit signal
	sigexit.sa_handler = &endInterface;
	sigaction(2, &sigexit, 0);
	repaint();
	
	getText();
}

int repaint(){
	//P(repaint);
	int i, u=0, marge = (dispUsers) ? 26 : 4;
	mvprintw(0, 0, "                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      ");
	
	// COLORS
	// COLOR_BLUE, COLOR_WHITE, COLOR_BLACK, COLOR_RED, COLOR_YELLOW, COLOR_GREEN, COLOR_MAGENTA, COLOR_CYAN, COLOR_GREEN,
	if(o_guicolors) start_color(); 		
	init_pair(C_BG, COLOR_WHITE, COLOR_BLUE);		// BG
	init_pair(C_CHAT, COLOR_BLACK, COLOR_WHITE);		// Chat plein text
	init_pair(C_CHAT_B, COLOR_BLACK, COLOR_WHITE);		// Chat border
	init_pair(C_CNCT, COLOR_WHITE, COLOR_BLACK);		// list text
	init_pair(C_CNCT_O, COLOR_WHITE, COLOR_BLACK);		// list border
	init_pair(C_PSEUDO, COLOR_BLUE, COLOR_WHITE);			// Pseudos
	init_pair(C_TOPINFOS, COLOR_YELLOW, COLOR_BLACK);		// Top infos
	wbkgd(stdscr, COLOR_PAIR(C_BG) | A_BOLD);		// BG COLORS
	
	// DIMENSIONS de la fenêtre
	getmaxyx(stdscr,LINES,COLS);
	refresh();
	
	// WHAT window
	wchato = newwin(LINES-5, COLS-marge, 2, 2); // cols, rows, beginY, beginX
	wchat = newwin(LINES-7, COLS-marge-2, 3, 3);	
	chatCols = COLS-marge-4-3;
	chatRows = LINES-3-5;
	box(wchato, 0, 0);
	mvwprintw(wchato, 1, 1, "");	// external window with border
	wbkgd(wchato, COLOR_PAIR(C_CHAT_B) | A_BOLD);
	wbkgd(wchat, COLOR_PAIR(C_CHAT));
	
	// MESSAGES
	for(i=getFirstLineOffset() ; i<=text->pos; i++){
		cutMessage(text->buf[i]);					// standard message
		wattron(wchat,COLOR_PAIR(C_PSEUDO) | A_BOLD);
		mvwprintw(wchat, u, 0, msgcut->pseudo);
		wattrset(wchat, 0);
		mvwprintw(wchat, u, (int)strlen(msgcut->pseudo)+1, msgcut->msg); 
		u += getNbLines("\0");						// \0 = msg into cut struct
	}
	wrefresh(wchato);										// CHAT	
	wrefresh(wchat);
	
	// USERS window
	if(dispUsers){
		attron(COLOR_PAIR(C_PSEUDO)); 
		wconnecto = newwin(LINES-5, 20, 2, COLS-23);
		wconnect = newwin(LINES-7, 18, 3, COLS-22);
		mvwprintw(wconnect, 0, 0, " - Participants -");
		for(i=0; i<users->nb; i++)
			mvwprintw(wconnect, i+1, 0, users->pseudo[i]);
		box(wconnecto, 0, 0);
	}
	
	
	// TOP INFOS
	attron(COLOR_PAIR(C_TOPINFOS)); 
	char m[12][20] = {"n",    "ew | Join [",    "b",    "] || whisper [",    "x",    
					"] | clea",    "r",    " || ",    "H",    "ELP | ",    "e",    "xit "};
	u=COLS-53;							// xi
	for(i=0; i<12; i++){
		wattrset(stdscr, (i%2 == 0) ? A_REVERSE : A_BOLD);
		mvprintw(0, u, "%s", m[i]);
		u += strlen(m[i]);
	}
	wattrset(stdscr, A_BOLD);
	u=0;
	
	
	// pseudo + channel name
	wattrset(stdscr, A_BOLD);
	mvprintw(0, 0, "[%s] ", pseudo);
	wattrset(stdscr, 0);
	mvprintw(0, (int)strlen(pseudo) +3, "chan:");
	wattrset(stdscr, A_REVERSE);
	mvprintw(0, (int)strlen(pseudo) +8, " %s ", users->chan);
	wattrset(stdscr, A_BOLD);
	
	
	// COLORS + REFRESH
	if(dispUsers){											// USERS
		wbkgd(wconnecto, COLOR_PAIR(C_CNCT_O));
		wbkgd(wconnect, COLOR_PAIR(C_CNCT));
		wrefresh(wconnecto);	
		wrefresh(wconnect);
	}
}


int getFirstLineOffset(){
	int pos = text->pos;
	int rest = chatRows;
	
	while (rest > 0){
		rest -= getNbLines(text->buf[pos--]);	// pos-- et nblines -= nombre de lignes occupés
		if(pos < 0){		// LOOP
			if(text->tours == 0) return 0;		// pas assez de msg pour remplir fenêtre (1ère tournée du buffer)
			else { 
				return 0;
			}
		}
	}
	
	// scroll verifs
	rest = pos+scr;
	if(pos+scr < 0){ scr++; return pos+scr; }
	else if(pos+scr > text->pos - chatRows/3){ scr--; return pos+scr; }
	else return rest;
}


int getNbLines(char txt[]){
	if(txt[0] == '\0')		// into msg cut struct
		return (int)(strlen(msgcut->pseudo) + strlen(msgcut->msg) + strlen(" "))
					/chatCols + 1;
	else 					// into 
		return (int)(strlen(txt))/chatCols + 1;
}


void popupGetText(const char txt[], const int len, int opt){
	int hplus = (opt == HELP) ? 11 : 0;
	int x = COLS/2 - POPUPWIN_WIDTH/2, 
		y = LINES/2 - (POPUPWIN_HEIGHT+hplus)/2;
	WINDOW *wgto, *wgt;
	
	wgto = newwin(POPUPWIN_HEIGHT+hplus, POPUPWIN_WIDTH, y, x);	
	wgt = newwin(POPUPWIN_HEIGHT+hplus-2, POPUPWIN_WIDTH-2, y+1, x+1);	
	box(wgto, 0, 0);
	mvwprintw(wgt, 1, 2, txt);	
	mvprintw(y+hplus+3, x+1, " > ");
	wrefresh(wgto);
	wrefresh(wgt);
	
	switch(opt){
		case HELP:
		case QUIT:
			rep[0] = getch();
			break;
		default:
			getnstr(rep, len);
	}
	
	repaint();
}


void getText(){
	int c, pos = 0;
	keypad(stdscr, TRUE);
	int whisper = 0, whisplen; 		// chuchoter = false

	repaint();
	
	while(1){
		// Display
		mvprintw(LINES - 2, 0, " Message :                                                                                                                                                                                                                                                                                                                                                                                                                             ", str);
		wattrset(stdscr, A_UNDERLINE);		mvprintw(LINES - 2, 1, "Message :", c);
		wattrset(stdscr, A_BOLD);			mvprintw(LINES - 2, 10, " ");
		if(whisper){ 
			mvprintw(LINES - 2, 11, "[TO:%s]", rep);
			whisplen = 5+strlen(rep);
		}
		
		for(c=0; c<pos; c++)
			mvprintw(LINES - 2, 11+whisplen+c, "%c", str[c]);
		
		// Get char
		c = getch();
		if(c > '0' && c < 'Z') goto key;		// normal char
		
		select: 
		switch(c){
			// SCROLL
			case KEY_UP: 		scr--; repaint(); break;		
			case KEY_DOWN: 		scr++; repaint(); break;
			
			// VALID
			case KEY_ENTER:
			case '\n': 
				str[pos] = '\0';
				if(strlen(str) == 0) break;				// empty
				if(whisper) sendMessage(WHISPER, strcat(strcat(rep, "#"), str));	// ex pat#msg to whisper
				else sendMessage(MSG, str);				// send
				pos = whisplen = whisper = 0;							// whisper finish after one msg
				usleep(10000);
				continue;
			
			// EXIT
			case 5: /*Alt+q*/	
				endInterface(); break; 
			
			// HELP
			case 8: /*Alt+h*/	
				popupGetText("           - - - -   H  E  L  P   - - - - \n\n [UP KEY]   Scroll up\n [DOWN KEY] Scroll down\n\n [c^N] Create a NEW channel\n [c^B] Join an existing channel\n [c^X] Private message to someone (in any channel)\n [c^K] Kick someone (must be admin or chan owner)\n\n [c^R] Clear messages (Remove)\n [c^E] EXIT chat & disconnect ", 1, HELP); 
				c = rep[0]; goto select; break;
			// Display users
			case 21: /*Alt+u*/
				dispUsers = !dispUsers; repaint(); break; 
			
			// CHAN (create)
			case 14:  /*Alt+c*/	
				popupGetText("Créer un nouveau chanel [enter to cancel] :\n", MAX_CHANNEL_LENGTH, STD); 
				if(rep[0] != '\n' && rep[0] != '\0') sendMessage(CREATE, rep); usleep(10000); break;
			// CHAN (join)
			case 2: /*Alt+j*/	
				popupGetText("Joindre quel chanel ? [enter to cancel] \n", MAX_CHANNEL_LENGTH, STD);
				if(rep[0] != '\n' && rep[0] != '\0') sendMessage(JOIN, rep); usleep(10000); break;
			
			// Whisper
			case 24: /*Alt+x*/
				popupGetText("Chuchoter à qui ? [enter to cancel] \n", MAX_PSEUDO_LENGTH, STD); 
				whisper = (rep[0] != '\n' && rep[0] != '\0') ? 1 : 0; break;
			
			// Kick
			case 11: /*Alt+K*/
				popupGetText("Kick who ? [enter to cancel] \n", MAX_CHANNEL_LENGTH, STD);
				if(rep[0] != '\n' && rep[0] != '\0') sendMessage(KICK, rep); usleep(10000); break;
			
			// CLEAR screen
			case 18: /*Alt+r*/	
				text->pos=text->tours = 0; strcpy(text->buf[0], TXT_NO_MESSAGE); repaint();
				break; 
			
			// BACK
			case 263: /* erase char*/
				pos = (pos > 0) ? pos-1 : 0; break;
				
			case -10: case -18: case -1: case -102: break;		// BANNED SIGNAL
			
			// Normal text
			default:
				key: 
				if(c > 200 ) break;
				if(pos < MAX_MSG_LENGTH) str[pos++] = c;
				//mvprintw(LINES - 1, 0, " CODE last char = %d  ++     ", str[pos-1]);
		}
	}
}


