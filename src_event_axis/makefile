AXIS_USABLE_LIBS = UCLIBC GLIBC
include $(AXIS_TOP_DIR)/tools/build/rules/common.mak

# Парсинг данных из package.conf
APP_NAME := $(shell grep "MENUNAME" package.conf | cut -d'=' -f2 | tr -d '"')
APP_VER  := $(shell grep "APPMAJORVERSION" package.conf | cut -d'=' -f2 | tr -d '"').$(shell grep "APPMINORVERSION" package.conf | cut -d'=' -f2 | tr -d '"').$(shell grep "APPMICROVERSION" package.conf | cut -d'=' -f2 | tr -d '"')
APP_DEFS  = -DAPP_NAME_STR=\"$(APP_NAME)\" -DAPP_VER_STR=\"$(APP_VER)\"

# Настройки кросс-компиляции
CXX = mipsisa32r2el-axis-linux-gnu-g++
STRIP = mipsisa32r2el-axis-linux-gnu-strip
LIBEVENT_DIR = $(CURDIR)/libs/build/libevent_mipsisa32r2el

# Флаги
COMMON_FLAGS = -Wall -Wextra -Werror -pedantic -std=c++98 \
               -I$(LIBEVENT_DIR)/include $(APP_DEFS)
LDFLAGS = -L$(LIBEVENT_DIR)/lib
LDLIBS = -levent -lpthread -lrt

TARGET = tixerver
SRC = main.cpp camera_info.cpp

all: debug

debug: CXXFLAGS = $(COMMON_FLAGS) -g -O0
debug: $(TARGET)

release: CXXFLAGS = $(COMMON_FLAGS) -O2
release: $(TARGET)
	$(STRIP) $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) $(LDFLAGS) $(LDLIBS) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all debug release clean
