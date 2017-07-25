CC = gcc
LD = gcc
CFLAGS = -Wall -Wextra -Werror -O3
LDFLAGS = -lskarnet
PREFIX = /usr/local
DESTDIR =

BINDIR = $(PREFIX)/bin

-include config.mak

.PHONY: clean install all test

HEADERS = src/common.h src/functions.h

SRCS := \
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

OBJS := ${SRCS:c=o}

TARGET = sockexec

all: $(TARGET)

install: $(TARGET)
	install -s -D -m 0755 bin/$(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)

$(TARGET): bin/$(TARGET)

bin/$(TARGET): $(OBJS)
	mkdir -p bin
	$(CC) -o bin/$(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f bin/$(TARGET)
	rm -f $(OBJS)

test: $(TARGET)
