
aptsh:
	cd src/ && make

clean:
	cd src && make clean

install:
	cp src/aptsh.1 $(DESTDIR)/usr/share/man/man1/
	cp src/aptsh $(DESTDIR)/usr/bin/
	cp src/aptsh.conf $(DESTDIR)/etc/
	if test ! -d $(DESTDIR)/usr/share/aptsh/; then  \
		mkdir -p $(DESTDIR)/usr/share/aptsh/;\
	fi
	cp src/ls $(DESTDIR)/usr/share/aptsh/
	cp src/rls $(DESTDIR)/usr/share/aptsh/

