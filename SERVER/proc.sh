#!/bin/bash

function pause(){
	echo
	read -p "$PRESSANYKEY"
}


echo "HELLO"


for (( c=0; c<$6 ; c++ )) ; do
	echo "PAYLOAD"
	read line
	echo $line >> ~/Documents/salida.txt	
done

echo $PWD >> ~/Documents/salida.txt

echo "host = \"$1\", ip = \"$2\", port = \"$3\", event_type = \"$4\", event = \"$5\", payload_lenght = \"$6\"" >> ~/Documents/salida.txt


if [ $4 -eq 1 ] ; then
	beep -l 30 -f 200 2>/dev/null >/dev/null
	#mariotune.sh
fi

if [ $4 -eq 2 ] ; then
	beep -l 30 -f 2400 2>/dev/null >/dev/null
	#mariotune.sh
fi

if [ $4  -eq 3 ] ; then
	beep -l 30 -f 1200 2>/dev/null >/dev/null
	#pkill mariotune.sh
fi

#echo "Command completed."

#pause

#sleep 1

echo "CLOSE"

exit 0
