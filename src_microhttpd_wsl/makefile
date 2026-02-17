# Makefile for axis server
# Before compiling install libmicrohttpd-dev

CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -std=c99 -O2
TARGET = server
SRC = main.c

# Если libevent установлена через sudo apt install libevent-dev, пути можно не указывать
LIB_DIR = /usr/lib/x86_64-linux-gnu
INC_DIR = /usr/include

# -lmicrohttpd (core), -lpthread (threads), -lrt (timers)
LDLIBS = -lmicrohttpd -lpthread -lrt

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -I$(INC_DIR) -o $(TARGET) $(SRC) -L$(LIB_DIR) $(LDLIBS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
