
aptsh: main.o genindex.o readindex.o apt_cmds.o string.o config_parse.o aptsh.1.gz
	gcc -lreadline -o aptsh main.o genindex.o readindex.o apt_cmds.o string.o config_parse.o 

main.o: main.c genindex.h readindex.h apt_cmds.h config_parse.h 
	gcc -c main.c -o main.o 

genindex.o: genindex.c genindex.h 
	gcc -c genindex.c -o genindex.o 

readindex.o: readindex.c readindex.h 
	gcc -c readindex.c -o readindex.o 

apt_cmds.o: apt_cmds.c apt_cmds.h config_parse.h genindex.h readindex.h
	gcc -c apt_cmds.c -o apt_cmds.o 

string.o: string.c string.h 
	gcc -c string.c -o string.o 

config_parse.o: config_parse.c config_parse.h 
	gcc -c config_parse.c -o config_parse.o 

aptsh.1.gz: aptsh.1
	cp aptsh.1 man-bak
	gzip aptsh.1
	mv man-bak aptsh.1
	touch aptsh.1.gz

clean:
	rm *.o
	rm aptsh
	rm aptsh.1.gz
