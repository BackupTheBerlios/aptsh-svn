
all: ls rls aptsh

ls: ls.c
	gcc -Wall ls.c -o ls

rls: rls.c
	gcc -Wall rls.c -o rls

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
	if test `find . -iname '*.o' | head -n 1` ; then  \
		rm *.o ; \
	fi
	if [ -x aptsh ] ; then \
		rm aptsh;\
	fi
	if [ -x ls ]; then\
		rm ls;\
	fi
	if [ -x rls ]; then\
		rm rls;\
	fi
	if [ -e aptsh.1.gz ]; then\
		rm aptsh.1.gz;\
	fi
