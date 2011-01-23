#/bin/bash
usersFile="users"

minLoginChars=4		    # +1 !!
minPasswdChars=4		# +1 !!
minPseudoChars=2		# +1 !!

# go relative script folder 
cd `echo $0 | grep "^.*/" -o`	

# Display users
displayUsersFile () {
	echo " - - - u - s - e - r - s - - - - - - - - "
	cat $usersFile
	echo " - - - - - - - - - - - - - - - - - - - - "
	read -p "Enter to continue" go
}

#######################################
# Add one user (pseudo, mdp, rank)
addUser () {
	while [ 1=1 ]; do
		#Read
		read -p " > Login ? " login
	    #stty -echo
		read -p " > Password ? " passwd
	    #stty echo ; echo
		read -p " > Pseudo ? " pseudo
		read -p " > administrateur ? [oy] " rank
		
		# conditions
		if [ `echo $login | wc -c` -lt $minLoginChars ] ; then
			echo "$minLoginChars characters minimum for login"
			continue
		elif [ `echo $passwd | wc -c` -lt $minPseudoChars ] ; then
			echo "$minPasswdChars minimum for password"
			continue
		elif [ `echo $pseudo | wc -c` -lt $minPseudoChars ] ; then
			echo "$minPseudoChars characters minimum for pseudonyms"
			continue
		fi
		break
	done
	
	# admin?
	if [ $rank = "y" ] || [ $rank = "o" ] ; then
		rank=2
	else
		rank=1
	fi
	
	# continue ?
	passwd=`./sfhash $passwd`		# hash
	line="$login:$passwd:$pseudo:$rank"
	read -p " > Write '$line', validate ? [y] " go
	
	#add line
	if [ $go = "y" ] ; then
		chmod +w $usersFile
		echo $line >> $usersFile
		chmod -w $usersFile
		echo " : writed"
		displayUsersFile
	fi
}



################################################
# REMOVE USER
removeUser () {
	read -p " > Login to remove ? " login
	read -p " > Will remove $login from user list, continue ? [y] " go
	if [ $go = "y" ] ; then
		res=`cat $usersFile | grep -v "^${login}:"`
		rm -f $usersFile
		echo $res | tr " " "\n" > $usersFile
		chmod -w $usersFile
		echo " : removed"
		displayUsersFile
	fi
}

################################################
# MENU
while [ 1=1 ]
do
	echo "##########################################"
	echo "####  U S E R S   M A N A G E M E N T ####"
	echo "# d/ Display users file"
	echo "# e/ Edit users file (vi)"
	echo "#  -  -  -  -  -  -  -  -  -  "
	echo "# a/ Add one user"
	echo "# r/ Remove one user"
	echo "#  -  -  -  -  -  -  -  -  -  "
	echo "# q/ Quit"

	read -p 'Your choice : ' choix

	case $choix in
		"d")
			displayUsersFile ;;
		"e") 
			chmod +w $usersFile
			vi $usersFile
			chmod -w $usersFile ;;
		"a")
			addUser ;;
		"r")
			removeUser ;;
		"q")
			exit 0
	esac
done
