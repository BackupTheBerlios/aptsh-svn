
all: ls rls aptsh

ls: ls.c
	gcc -Wall ls.c -o ls

rls: rls.c
	gcc -Wall rls.c -o rls

aptsh: main.o readindex.o apt_cmds.o string.o config_parse.o aptsh.1.gz
	g++ -g -O2 -Wall -lreadline -lapt-pkg -o aptsh main.o readindex.o apt_cmds.o string.o config_parse.o 

main.o: main.cc readindex.h apt_cmds.h config_parse.h 
	g++ -g -O2 -Wall -c main.cc -o main.o 

readindex.o: readindex.cc readindex.h 
	g++ -g -O2 -Wall -c readindex.cc -o readindex.o 

apt_cmds.o: apt_cmds.cc apt_cmds.h config_parse.h readindex.h
	g++ -g -O2 -Wall -c apt_cmds.cc -o apt_cmds.o 

string.o: string.c string.h 
	gcc -g -O2 -Wall -c string.c -o string.o 

config_parse.o: config_parse.c config_parse.h 
	gcc -g -O2 -Wall -c config_parse.c -o config_parse.o 

# <debug>
debug: aptsh-debug

aptsh-debug: main-debug.o readindex-debug.o apt_cmds-debug.o string-debug.o config_parse-debug.o
	g++ -g -ggdb -L/usr/lib/debug -lreadline -lapt-pkg -o aptsh-debug main-debug.o readindex-debug.o apt_cmds-debug.o string-debug.o config_parse-debug.o

main-debug.o: main.cc readindex.h apt_cmds.h config_parse.h
	g++ -g -ggdb -c main.cc -o main-debug.o

readindex-debug.o: readindex.cc readindex.h
	g++ -g -ggdb -c readindex.cc -o readindex-debug.o

apt_cmds-debug.o: apt_cmds.cc apt_cmds.h config_parse.h readindex.h
	g++ -g -ggdb -c apt_cmds.cc -o apt_cmds-debug.o

string-debug.o: string.c string.h
	gcc -g -ggdb -c string.c -o string-debug.o

config_parse-debug.o: config_parse.c config_parse.h
	gcc -g -ggdb -c config_parse.c -o config_parse-debug.o

# </debug>

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
