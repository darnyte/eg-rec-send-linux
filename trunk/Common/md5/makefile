#MD5 Self test.


debug: md5main-debug.o md5-debug.o
	gcc -Wall -ansi -o md5-self-test md5main-debug.o md5-debug.o -lm -DDEBUG -DNDEBUG

release: md5main.o md5.o

	gcc -Wall -ansi -o md5-self-test md5main.o md5.o -lm

###
md5main.o: md5main.c md5.h
	gcc -Wall -ansi -O2 -c md5main.c -o md5main.o

md5main-debug.o: md5main.c md5.h
	gcc -g -Wall -ansi -O0 -c md5main.c -o md5main-debug.o -DDEBUG -DNDEBUG
###

###
md5.o: md5.c md5.h
	gcc -Wall -ansi -O2 -c md5.c -o md5.o

md5-debug.o: md5.c md5.h
	gcc -g -Wall -ansi -O0 -c md5.c -o md5-debug.o -DDEBUG -DNDEBUG
###

clean:
	rm -f md5-self-test *.o





