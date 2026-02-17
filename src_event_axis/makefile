# Path to SDK Axis (normally makes in 'source init_env')
AXIS_USABLE_LIBS = UCLIBC GLIBC
include $(AXIS_TOP_DIR)/tools/build/rules/common.mak

CC = mipsisa32r2el-axis-linux-gnu-gcc

LIBEVENT_DIR = $(CURDIR)/libs/build/libevent_mipsisa32r2el

CFLAGS = -Wall -Wextra -Werror -pedantic -std=c99 -I$(LIBEVENT_DIR)/include
LDFLAGS = -L$(LIBEVENT_DIR)/lib -static
LDLIBS = -levent -lpthread -lrt

TARGET = server
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) $(LDLIBS) -o $(TARGET)

clean:
	rm -f $(TARGET)
