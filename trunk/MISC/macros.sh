#!/bin/sh
tmpfile=/var/tmp/foo.c
touch $tmpfile
gcc -E -dM $tmpfile
rm $tmpfile
