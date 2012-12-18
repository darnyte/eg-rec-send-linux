#!/bin/sh

if [ -z "$1" ]; then
	exit 1
fi

HEADER_FILE="$1"

if [ -f "$HEADER_FILE" ]; then
	echo "" > "$HEADER_FILE"
else
	touch  "$HEADER_FILE"
fi

if [ $? -ne 0 ] ; then
	exit 2
fi

UNAME_PLATFORM=true

if [ ! -z "$2" ]; then
	if [ "$2" -eq "$2" > /dev/null 2> /dev/null ]; then
		if [ "$2" -ne 0 ]; then
			UNAME_PLATFORM=false
			beep -l 333 2000
		fi
	else
		exit 3
	fi
fi

if $UNAME_PLATFORM ; then

	KERNEL_UNAME="`uname -s`"
	NODE_UNAME="`uname -n`"
	KERNEL_RELEASE_UNAME="`uname -r`"
	KERNEL_VERSION_UNAME="`uname -v`"
	MACHINE_UNAME="`uname -m`"
	PROCESSOR_UNAME="`uname -p`"
	HARDWARE_UNAME="`uname -i`"
	OS_UNAME="`uname -o`"

fi

if [ $? -ne 0 ] ; then
	exit 4
fi

echo "#ifndef _MYPLATFORM_H_INCLUDED_" >> "$HEADER_FILE"
echo "#define _MYPLATFORM_H_INCLUDED_" >> "$HEADER_FILE"
echo >> "$HEADER_FILE"
echo >> "$HEADER_FILE"

if $UNAME_PLATFORM ; then

	echo "#define UNAME_PLATFORM 1" >> "$HEADER_FILE"
	echo "#define KERNEL_UNAME \"$KERNEL_UNAME\"" >> "$HEADER_FILE"
	echo "#define NODE_UNAME \"$NODE_UNAME\"" >> "$HEADER_FILE"
	echo "#define KERNEL_RELEASE_UNAME \"$KERNEL_RELEASE_UNAME\"" >> "$HEADER_FILE"
	echo "#define KERNEL_VERSION_UNAME \"$KERNEL_VERSION_UNAME\"" >> "$HEADER_FILE"
	echo "#define MACHINE_UNAME \"$MACHINE_UNAME\"" >> "$HEADER_FILE"
	echo "#define PROCESSOR_UNAME \"$PROCESSOR_UNAME\"" >> "$HEADER_FILE"
	echo "#define HARDWARE_UNAME \"$HARDWARE_UNAME\"" >> "$HEADER_FILE"
	echo "#define OS_UNAME \"$OS_UNAME\"" >> "$HEADER_FILE"

else

	echo "#ifdef UNAME_PLATFORM" >> "$HEADER_FILE"
	echo "#undef UNAME_PLATFORM" >> "$HEADER_FILE"
	echo "#endif" >> "$HEADER_FILE"

fi

echo >> "$HEADER_FILE"
echo >> "$HEADER_FILE"
echo "#endif /* _MYPLATFORM_H_INCLUDED_ */" >> "$HEADER_FILE"
echo >> "$HEADER_FILE"

exit 0

