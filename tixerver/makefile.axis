# Makefile for building tixerver for Axis only
# Usage: make ARCH=MIPS SN=00408C123456 release

# 0. Настройки по умолчанию
ARCH ?= MIPS
SN ?= free
.DEFAULT_GOAL := release_default

# 1. Проверка архитектуры
SUPPORTED_ARCHS = MIPS ARMV7HF
ifeq ($(filter $(ARCH),$(SUPPORTED_ARCHS)),)
  $(error Invalid ARCH: "$(ARCH)". Please use ARCH=MIPS or ARCH=ARMV7HF)
endif

# 2. Проверка серийного номера (SN)
ifneq ($(SN), free)
  ifeq ($(shell echo "$(SN)" | grep -xE '[0-9a-fA-F]{12}'),)
    $(error Invalid SN: "$(SN)". Must be 'free' or a 12-digit hex number)
  endif
endif

export AXIS_TOP_DIR = /home/sergebabayan.linux/axis/emb-app-sdk_2_0_3
AXIS_USABLE_LIBS = UCLIBC GLIBC

# Подключаем правила Axis
include $(AXIS_TOP_DIR)/tools/build/rules/common.mak

# 3. Локальные пути
CJSON_DIR = /Users/sergebabayan/vscode-workspace/tix-server/tixerver/libs/cjson

# Поддиректории для VPATH и Includes
SUBDIRS = src src/common src/capture src/network src/model src/process
VPATH = $(SUBDIRS)
INC_FLAGS = $(addprefix -I, $(SUBDIRS))

# 4. Данные приложения
APP_NAME := $(shell grep "APPNAME" package.conf | cut -d'=' -f2 | tr -d '"' | tr -d '\r')
MAJOR := $(shell grep "APPMAJORVERSION" package.conf | cut -d'=' -f2 | tr -d '"' | tr -d '\r')
MINOR := $(shell grep "APPMINORVERSION" package.conf | cut -d'=' -f2 | tr -d '"' | tr -d '\r')
MICRO := $(shell grep "APPMICROVERSION" package.conf | cut -d'=' -f2 | tr -d '"' | tr -d '\r')
APP_VER := $(MAJOR).$(MINOR).$(MICRO)

# Макросы (добавлен SERIAL_NUMBER)
APP_DEFS = -DAPP_NAME_STR=\"$(APP_NAME)\" -DAPP_VER_STR=\"$(APP_VER)\" \
           -DSERIAL_NUMBER=\"$(SN)\" -DCAMERA_AXIS

# Список общих объектов
COMMON_OBJECTS = main.o camera_info.o network_server.o math_engine.o \
                 command_processor.o rowMatClass.o adapter.o \
                 parser_config.o capture_factory.o cJSON.o capture_axis.o

# Логика выбора архитектуры
ifeq ($(ARCH), MIPS)
    LIBEVENT_DIR = /Users/sergebabayan/vscode-workspace/tix-server/tixerver/libs/build.mips
    OPENCV_DIR = /home/sergebabayan.linux/opencv341/build.mipsisa32r2el/install
    ARCH_FLAGS = -msoft-float -march=mips32r2 -EL
    OBJECTS = $(COMMON_OBJECTS)
else ifeq ($(ARCH), ARMV7HF)
    LIBEVENT_DIR = /Users/sergebabayan/vscode-workspace/tix-server/tixerver/libs/build.armv7hf
    OPENCV_DIR = /home/sergebabayan.linux/opencv341/build.armv7hf/install
    APP_DEFS += -DTraffiX_TRACKER
    SUBDIRS += src/tracker
    VPATH += src/tracker
    INC_FLAGS += -Isrc/tracker
    TRACKER_SRC = $(wildcard src/tracker/*.cpp)
    OBJECTS = $(COMMON_OBJECTS) $(notdir $(TRACKER_SRC:.cpp=.o))
    ARCH_FLAGS = 
endif

SPECIFIC_LDLIBS = -lcapture

# 5. ФОРМИРОВАНИЕ ФЛАГОВ
COMMON_FLAGS = $(APP_DEFS) $(INC_FLAGS) -Wall -Wextra -Werror -pedantic \
               -I$(LIBEVENT_DIR)/include \
               -I$(CJSON_DIR) \
               -I$(OPENCV_DIR)/include

CXXFLAGS := $(filter-out -std=c89 -std=c90 -ansi, $(CXXFLAGS))
CXXFLAGS += $(COMMON_FLAGS) $(ARCH_FLAGS) -std=c++11
CFLAGS   += $(COMMON_FLAGS) $(ARCH_FLAGS) -std=c89

# 6. ЛИНКОВКА
LDFLAGS := -L$(LIBEVENT_DIR)/lib -L$(OPENCV_DIR)/lib -L$(OPENCV_DIR)/share/OpenCV/3rdparty/lib $(LDFLAGS)
LDFLAGS += -L$(prefix)/lib -L$(prefix)/usr/lib
LDFLAGS += -Wl,-rpath-link,$(prefix)/usr/lib:$(prefix)/lib

OPENCV_MIN_LIBS = -lopencv_core -lzlib
LDLIBS = -levent $(OPENCV_MIN_LIBS) -lpthread $(SPECIFIC_LDLIBS) -lrt -lm -ldl $(LIBS)

TARGET = tixerver

# --- ЦЕЛИ СБОРКИ ---
release_default: clean release

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

.PHONY: all debug release clean release_default
