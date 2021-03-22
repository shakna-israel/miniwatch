OUTFILE=miniwatch
PREFIX=/usr/local/bin

all:
	$(CC) -g -o $(OUTFILE) src/main.c

install:
	install -s $(OUTFILE) -t $(PREFIX)/
