all: mergetars

mergetars:
	cc -c -std=c99 -Wall -Werror filelist.c
	cc -o filelist filelist.o
	cc -c -std=c99 -Wall -Werror main.c
	cc -o mergetars main.o


clean:
	rm *.o
	rm mergetars
	rm filelist
