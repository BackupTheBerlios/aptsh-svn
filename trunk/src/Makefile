
aptsh: main.o readindex.o apt_cmds.o string.o config_parse.o aptsh.1.gz
	g++ -g -O2 -lreadline -lapt-pkg -o aptsh main.o readindex.o apt_cmds.o string.o config_parse.o 

main.o: main.c readindex.h apt_cmds.h config_parse.h 
	gcc -g -O2 -c main.c -o main.o 

readindex.o: readindex.cc readindex.h 
	g++ -g -O2 -c readindex.cc -o readindex.o 

apt_cmds.o: apt_cmds.c apt_cmds.h config_parse.h readindex.h
	gcc -g -O2 -c apt_cmds.c -o apt_cmds.o 

string.o: string.c string.h 
	gcc -g -O2 -c string.c -o string.o 

config_parse.o: config_parse.c config_parse.h 
	gcc -g -O2 -c config_parse.c -o config_parse.o 

aptsh.1.gz: aptsh.1
	cp aptsh.1 man-bak
	gzip aptsh.1
	mv man-bak aptsh.1
	touch aptsh.1.gz

clean:
	rm *.o
	rm aptsh
	rm aptsh.1.gz
