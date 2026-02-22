AXIS_USABLE_LIBS = UCLIBC GLIBC
include $(AXIS_TOP_DIR)/tools/build/rules/common.mak

# Get data from package.conf
APP_NAME := $(shell grep "MENUNAME" package.conf | cut -d'=' -f2 | tr -d '"')
MAJOR := $(shell grep "APPMAJORVERSION" package.conf | cut -d'=' -f2 | tr -d '"')
MINOR := $(shell grep "APPMINORVERSION" package.conf | cut -d'=' -f2 | tr -d '"')
MICRO := $(shell grep "APPMICROVERSION" package.conf | cut -d'=' -f2 | tr -d '"')
APP_VER := $(MAJOR).$(MINOR).$(MICRO)
APP_DEFS = -DAPP_NAME_STR=\"$(APP_NAME)\" -DAPP_VER_STR=\"$(APP_VER)\"

CXX = mipsisa32r2el-axis-linux-gnu-g++
CC = mipsisa32r2el-axis-linux-gnu-gcc
STRIP = mipsisa32r2el-axis-linux-gnu-strip

LIBEVENT_DIR = $(CURDIR)/libs/build/libevent_mipsisa32r2el
CJSON_DIR = $(CURDIR)/libs/cjson

COMMON_FLAGS = -Wall -Wextra -Werror -pedantic \
               -I$(LIBEVENT_DIR)/include -I$(CJSON_DIR) $(APP_DEFS)

CXXFLAGS = $(COMMON_FLAGS) -std=c++98
CFLAGS = $(COMMON_FLAGS) -std=c89

LDFLAGS = -L$(LIBEVENT_DIR)/lib
LDLIBS = -levent -lpthread -lcapture -lrt -lm

TARGET = tixerver

CPPSRC = main300.cpp camera_info.cpp network300_server.cpp math300_engine.cpp command300_processor.cpp
CSRC = $(CJSON_DIR)/cJSON.c

OBJECTS = main300.o camera_info.o network300_server.o math300_engine.o command300_processor.o cJSON.o

all: debug

debug: CXXFLAGS += -g -O0
debug: CFLAGS += -g -O0
debug: $(TARGET)

release: CXXFLAGS += -O2
release: CFLAGS += -O2
release: $(TARGET)
	$(STRIP) $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) $(LDLIBS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

cJSON.o: $(CJSON_DIR)/cJSON.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)

.PHONY: all debug release clean
