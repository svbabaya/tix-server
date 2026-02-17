# camera-server

# How to fix error with old version tar settings in AXIS script eap-create.sh:

tar czf $tarb --exclude="*~" --exclude="CVS" --format=gnu $APPNAME $ADPPACKCFG $ADPPACKPARAMCFG \
                $POSTINSTALLSCRIPT $OTHERFILES $HTTPD_CONF_LOCAL_FILES \
                $HTTPD_MIME_LOCAL_FILES $HTMLDIR $EVENT_DECLS_DIR $LIBDIR \
                $LUAPKGFILES $HTTPCGIPATHS

# The simple version for building server with prebuilding for Axis libevent. Don't forget add real-time library for libevent: -lrt

AXIS_USABLE_LIBS = UCLIBC GLIBC
include $(AXIS_TOP_DIR)/tools/build/rules/common.mak

CC = mipsisa32r2el-axis-linux-gnu-gcc

LIBEVENT_DIR = $(CURDIR)/libs/build/libevent_mipsisa32r2el

CFLAGS = -Wall -Wextra -Werror -pedantic -std=c99 -I$(LIBEVENT_DIR)/include
LDFLAGS = -L$(LIBEVENT_DIR)/lib
LDLIBS = -levent -lpthread -lrt

TARGET = server
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) $(LDLIBS) -o $(TARGET)

clean:
	rm -f $(TARGET)

# The current version of makefile for building server with prebuilding for Axis libevent.
# How to use: $ make clean debug or $ make clean release and $ create-package.sh mipsisa32r2el. Or $ make clean only.

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


# The true type of app after compiling
$ file server
server: ELF 32-bit LSB executable, MIPS, MIPS32 rel2 version 1 (SYSV), dynamically linked, interpreter /lib/ld.so.1, for GNU/Linux 2.6.29, with debug_info, not stripped