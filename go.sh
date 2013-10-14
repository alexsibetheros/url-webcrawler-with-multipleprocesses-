echo
echo
echo
echo "Compiling..."

LOOPS=${1:-"1"}
CHOICE=${2:-"1"}
rm -f out_file
execute="./mycrawler -f url_file -p 10 -s out_file -n 100"
execute2="./mycrawler -u htttp://www.in.gr -p 5 -s out_file -n 50"
if [ "$1" == "-d" ]; then
	make debug
else
	make
fi

if [ "$?" -eq "0" ]; then

	if [ $CHOICE == "1" ]; then
		exec=$execute
 	else
 		exec=$execute2
 	fi
 
 	echo $exec 	
 	if [ "$1" == "-d" ]; then
  		echo "Debuging..."
  		echo "----"
   		valgrind --tool=memcheck  --leak-check=full --track-origins=yes --show-reachable=yes ${exec}
	 	echo "----"
 		echo "END"
 	else	
 		for (( i=0 ; i<LOOPS ; i++ ))
 	 	do
   			echo "Running... Loop number: " $i
			echo "----"
   			$exec
   			echo "----"
 			echo "END"
  		done
 	fi
fi
# make clean
echo

