AXIS_USABLE_LIBS = UCLIBC GLIBC
include $(AXIS_TOP_DIR)/tools/build/rules/common.mak

CC = mipsisa32r2el-axis-linux-gnu-gcc
STRIP = mipsisa32r2el-axis-linux-gnu-strip

LIBEVENT_DIR = $(CURDIR)/libs/build/libevent_mipsisa32r2el

# Common flags
COMMON_FLAGS = -Wall -Wextra -Werror -pedantic -std=c99 -I$(LIBEVENT_DIR)/include
LDFLAGS = -L$(LIBEVENT_DIR)/lib
LDLIBS = -levent -lpthread -lrt

TARGET = server
SRC = main.c

# Debug as default
all: debug

# DEBUG: add -g (debugging) and turn off optimization
debug: CFLAGS = $(COMMON_FLAGS) -g -O0
debug: $(TARGET)
	@echo "--- DEBUG BUILD FINISHED ---"

# RELEASE: Add -O2 (optinization) and make strip
release: CFLAGS = $(COMMON_FLAGS) -O2
release: $(TARGET)
	$(STRIP) $(TARGET)
	@echo "--- RELEASE BUILD FINISHED (STRIPPED) ---"

# Building rules
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) $(LDLIBS) -o $(TARGET)

# Clean
clean:
	rm -f $(TARGET)
	@echo "--- CLEAN FINISHED ---"

.PHONY: all debug release clean
