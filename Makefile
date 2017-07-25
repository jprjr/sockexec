CC = gcc
LD = gcc
CFLAGS = -Wall -Wextra -Werror -O3
LDFLAGS = -lskarnet
PREFIX = /usr/local
DESTDIR =

BINDIR = $(PREFIX)/bin

-include config.mak

.PHONY: clean install all test

SRCS = \
	src/common.h \
	src/accept_client.c \
	src/child_read.c \
	src/child_spawn3.c \
	src/child_write.c \
	src/cleanup.c \
	src/client_read.c \
	src/client_write.c \
	src/close_connection.c \
	src/dump_connection.c \
	src/dump_fds.c \
	src/kill_processes.c \
	src/process_signals.c \
	src/route_event.c \
	src/sockexec.c \
	src/update_child.c \
	src/update_client.c

TARGET = sockexec

all: $(TARGET)

install: $(TARGET)
	install -s -D -m 0755 bin/$(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)

$(TARGET): bin/$(TARGET)

bin/$(TARGET): $(SRCS)
	mkdir -p bin
	$(CC) $(CFLAGS) -o bin/$(TARGET) $(SRCS) $(LDFLAGS)

clean:
	rm -f bin/$(TARGET)

test: $(TARGET)
