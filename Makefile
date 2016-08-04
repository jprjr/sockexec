CC = gcc
LD = gcc
CFLAGS = -Wall -Wextra -Werror -O3
LDFLAGS = -lskarnet
PREFIX = /usr/local
DESTDIR =

BINDIR = $(PREFIX)/bin

-include config.mak

.PHONY: clean install

SRCS = $(wildcard src/*.c)

TARGET = sockexec

all: $(TARGET)

install: $(TARGET)
	install -s -D -m 0755 bin/$(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)

$(TARGET): $(SRCS)
	mkdir -p bin
	$(CC) $(CFLAGS) -o bin/$(TARGET) $(SRCS) $(LDFLAGS)

clean:
	rm -f bin/$(TARGET)

