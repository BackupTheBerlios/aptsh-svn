
aptsh:
	cd src/ && make

clean:
	cd src && make clean

install:
	cp src/aptsh.1 $(DESTDIR)/usr/man/man1/
	cp src/aptsh $(DESTDIR)/usr/bin/
	cp src/aptsh.conf $(DESTDIR)/etc/
