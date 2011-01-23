#/bin/bash
server="chatserver"
client="chatclient"

echo $clientFolder
echo "######## CHAT INSTALLATION ########"

##################
# compiled ?
if [ ! -f $server ] || [ ! -f $client ] ; then
	echo "Please type 'make install' to compil executable, then install."
	exit
fi

##################
# RIGHTS ?
if [ ! -x "/root" ] ; then
	echo "# Warning, SECURITY !"
	echo "   > You are not logged as root user. Server can't be securised from clients logged as the same account as server + client list protection."
	echo "   > Recommended to give root privileges for server."
	
	# ask root privileges
	read -p "Ask root privileges [oy] ? " rep
	if [ ! -z $rep ] ; then
		if [ $rep = "o" ] || [ $rep = "y" ] ; then
			echo "# Reload install with sudo "
			exit
		fi			
	else
		echo "\n# Installation as normal user..."
		break;
	fi
	root=0
else
	echo "   > You have root privileges"
	root=1
fi

##################
# FOLDER ?
if [ $root = 1 ] ; then
	path="/usr/local/bin"
else
	path=$HOME
fi

# select another
echo " > Chat will be installed into $path"
while [ 1 ] ; do
	read -p "   Select another folder [blank if ok] : " path2
	if [ ! -z $path2 ] ; then
		if [ ! -e $path2 ] ; then
			read -p " /!\\ This is not a folder : $path2. > Want to create it during install [oy] ? " rep
			if [ $rep="o" ] || [ $rep="y" ] ; then
				path=$path2
				break
			fi
		elif [ ! -w $path2 ] ; then
			echo " /!\\ You haven't enought rights to write in $path2"
		else
			path=$path2
			break
		fi
	else
		break
	fi
done


###################
# CONTINUE ?
p="${path}/"				# go binaries
clientFileTO=`cat headers/config.h | grep "USR_FILE" | grep -o "\".*\"" | sed "s/\"//g" | sed "s/users$//g"`
if [ `echo $clientFileTO | grep "^.." -o | wc -c` -ne 3 ] ; then		# relative path, from chat folder
	clientFileTO=$p			# go configs
fi
## Correct relativ path (chatpipes/ folder)
if [ `echo $clientFileTO | grep "^.." -o | wc -c` -gt 1 ] ; then		# relative path, from chat folder
	clientFileTO=$path
fi

echo    " > Then install : binaries       in $path"
read -p "                  client admin   in $clientFileTO,    press enter to continue " rep
source=`pwd`


######### INSTALL (copy) ################
# create (if nec) bin directory, puts bins
mkdir -p $p
cp ${source}/$server $p		# copy bins
cp ${source}/$client $p

# create (if nec) users dir, puts manage/ users files
mkdir -p $p
cp ${source}/users_manag.sh $clientFileTO
cp ${source}/sfhash $clientFileTO
chmod +x $clientFileTO/sfhash 
if [ -e $source/users ] ; then		# copy users if one ever created
	cp ${source}/users $clientFileTO
else
	touch $clientFileTO/users
fi

# ROOT
chmod 400 $clientFileTO/users
chmod 100 $clientFileTO/$server



echo "## Installation cleared, server and client binaries copied into $path/"
echo "##  -> Run users_manag.sh to add/ remove clients"
echo "##  -> Then run server before running clients"



	

