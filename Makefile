
#########################
# Pour compiler des fichiers se servant de <curses.h> il faudra compiler
# avec en précisant la librairie -lcurses et installer libncurses5-dev
#########################
compil=sfhash chatserver chatclient 
ofolders=fclient/ fclient/gui/ fserv/ tools/ tools/SFhash/*.o
all : ${compil}
renew : mrproper all


# programmes (main)
chatserver : server.c fserv/serv_connect.o fserv/serv_recept.o fserv/serv_structs.o tools/ipctools.o tools/SFhash/SFhash.o  tools/io_extend.o
	gcc $^ -o $@

chatclient : client.c tools/io_extend.o fclient/client_connex.o fclient/client_chat.o fclient/client_struct.o -lcurses fclient/gui/draw.o tools/ipctools.o
	gcc $^ -o $@

# dépendances client
fclient/client_connex.o: fclient/client_connex.c
	@echo "\n __________ C  L  I  E  N  T _____________________________"
	gcc -c $^ -o $@
fclient/client_chat.o: fclient/client_chat.c 
	gcc -c $^ -o $@
fclient/client_struct.o: fclient/client_struct.c 
	gcc -c $^ -o $@
fclient/gui/draw.o: fclient/gui/draw.c
	gcc -c $^ -lcurses -o $@
fclient/gui/draw_capture.o: fclient/gui/draw_capture.c
	gcc -c $^ -lcurses -o $@


# dépendances server
fserv/serv_recept.o: fserv/serv_recept.c
	gcc -c $^ -o $@
fserv/serv_connect.o: fserv/serv_connect.c
	@echo "\n __________ S  E  R  V  E  R _____________________________"
	gcc -c $^ -o $@
fserv/serv_structs.o: fserv/serv_structs.c
	gcc -c $^ -o $@



# dépendances mixtes:
tools/ipctools.o: tools/ipctools.c
	@echo "\n __________ T O O L S _____________________________"
	gcc -c $^ -o $@
tools/io_extend.o: tools/io_extend.c
	gcc -c $^ -o $@

sfhash:
	@echo "\n __________ H A S H _____________________________"
	cd tools/SFhash/ \
	$(MAKE) -f Makefile \
	cp sfhash ../../

##########################
# # # C L E A N I N G
# remove useless (temp) files
clean:
	@echo " ___________ C  L  E  A  N _____________________________"
	cd tools/SFhash/ \
	$(MAKE) mrproper -f Makefile
	rm -f fclient/*.o
	rm -f fclient/gui/*.o
	rm -f fserv/*.o
	rm -f tools/*.o
	rm -f tools/SFhash/*.o
	

# Remove all generated file (*.o + executable) -> like orginal folder
mrproper: clean
	rm -f ${compil}
	
install:
	./install.sh


#########################
# H E L P
help:
	@echo " __________ H E L P _____________________________"
	@echo " > make [all]      : to compil"
	@echo " > make install    : After a make, copy final binaries"
	@echo " > make clean      : to remove all transitive files (*.o)"
	@echo " > make mrproper   : remove all generated make files"
	@echo " > make renew      : clean and make"



#########################
# C O N F I G
configure:
	@echo " __________ C O N F I G U R E ______________________"
	@echo " > Before compile, you can choose absolute path to put users config, to separate from binaries"
	@echo "                   >>> open headers/config.h and edit USR_FILE <<<"
	@echo " > Binaries dir will be ask at installation"


#########################
# A L I A S
a: all
r: renew
c: clean
m: mrproper
i: install
cf: configure
