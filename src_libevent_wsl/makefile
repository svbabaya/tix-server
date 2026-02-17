# Makefile for axis server
# Before compiling install libevent-dev and pkg-config:
# $ sudo apt update && sudo apt install pkg-config libevent-dev
CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -std=c99
TARGET = server
SRC = main.c

# Check does libevent exist using pkg-config
LIB_CHECK := $(shell pkg-config --exists libevent && echo yes || echo no)

ifeq ($(LIB_CHECK),no)
  $(error Library 'libevent' not found! Run: sudo apt install libevent-dev)
endif

# Get flags for linking
LDFLAGS = $(shell pkg-config --libs libevent)

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)
