OUTFILE=miniwatch
PREFIX=/usr/local/bin

all:
	$(CC) -g -o $(OUTFILE) $(CFLAGS) src/main.c

install:
	install -s $(OUTFILE) -t $(PREFIX)/
