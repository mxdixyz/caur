CC        = gcc
CFLAGS    = -O3 -lgit2
PREFIX    = /usr/local
MANPREFIX = $(PREFIX)/share/man

all:
	$(CC) $(CFLAGS) -c caur.c
	$(CC) $(CFLAGS) -o caur caur.o

install: all
	mkdir -p "$(PREFIX)/bin"
	cp -f caur "$(PREFIX)/bin"
	chmod 755 "$(PREFIX)/bin/caur"

	mkdir -p "$(MANPREFIX)/man1"
	cp -f caur.1 "$(MANPREFIX)/man1"
	chmod 644 "$(MANPREFIX)/man1/caur.1"

uninstall:
	rm -f "$(PREFIX)/bin/caur" "$(MANPREFIX)/man1/caur.1"

clean:
	rm -f caur caur.o