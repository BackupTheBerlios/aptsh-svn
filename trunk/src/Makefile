
all: ls rls aptsh

ls: ls.c
	gcc -Wall ls.c -o ls

rls: rls.c
	gcc -Wall rls.c -o rls

aptsh: main.o readindex.o apt_cmds.o string.o config_parse.o column.o dpkg_complete.o aptsh.1.gz
	g++ -g -O2 -Wall -lreadline -lapt-pkg -o aptsh main.o readindex.o apt_cmds.o string.o config_parse.o column.o dpkg_complete.o

main.o: main.cc readindex.h apt_cmds.h config_parse.h 
	g++ -g -O2 -Wall -c main.cc -o main.o 

readindex.o: readindex.cc readindex.h 
	g++ -g -O2 -Wall -c readindex.cc -o readindex.o 

apt_cmds.o: apt_cmds.cc apt_cmds.h config_parse.h readindex.h
	g++ -g -O2 -Wall -c apt_cmds.cc -o apt_cmds.o 

string.o: string.cc string.h 
	g++ -g -O2 -Wall -c string.cc -o string.o 

config_parse.o: config_parse.c config_parse.h 
	gcc -g -O2 -Wall -c config_parse.c -o config_parse.o

column.o: column.cc column.h
	g++ -g -O2 -Wall -c column.cc -o column.o

dpkg_complete.o: dpkg_complete.cc dpkg_complete.h
	g++ -g -O2 -Wall -c dpkg_complete.cc -o dpkg_complete.o


# <debug>
debug: aptsh-debug

aptsh-debug: main-debug.o readindex-debug.o apt_cmds-debug.o string-debug.o config_parse-debug.o column-debug.o dpkg_complete-debug.o
	g++ -g -ggdb -L/usr/lib/debug -lreadline -lapt-pkg -o aptsh-debug main-debug.o readindex-debug.o apt_cmds-debug.o string-debug.o config_parse-debug.o column-debug.o dpkg_complete-debug.o

main-debug.o: main.cc readindex.h apt_cmds.h config_parse.h
	g++ -g -ggdb -c main.cc -o main-debug.o

readindex-debug.o: readindex.cc readindex.h
	g++ -g -ggdb -c readindex.cc -o readindex-debug.o

apt_cmds-debug.o: apt_cmds.cc apt_cmds.h config_parse.h readindex.h
	g++ -g -ggdb -c apt_cmds.cc -o apt_cmds-debug.o

string-debug.o: string.cc string.h
	g++ -g -ggdb -c string.cc -o string-debug.o

config_parse-debug.o: config_parse.c config_parse.h
	gcc -g -ggdb -c config_parse.c -o config_parse-debug.o

column-debug.o: column.h column.cc
	g++ -g -ggdb -c column.cc -o column-debug.o

dpkg_complete-debug.o: dpkg_complete.h dpkg_complete.cc
	g++ -g -ggdb -c dpkg_complete.cc -o dpkg_complete-debug.o

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
