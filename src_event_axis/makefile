# Makefile for building tixerver. Using: $ make [ clean | debug | release ] or $ make [ clean debug | clean release ]
export AXIS_TOP_DIR = /home/babay/axis_sdk/axis/emb-app-sdk_2_0_3
AXIS_USABLE_LIBS = UCLIBC GLIBC

# 1. Подключаем правила Axis
include $(AXIS_TOP_DIR)/tools/build/rules/common.mak

# 2. Ваши локальные пути
LIBEVENT_DIR = $(CURDIR)/libs/build/libevent_mipsisa32r2el
CJSON_DIR = $(CURDIR)/libs/cjson
OPENCV_DIR = /mnt/c/viat/libs/opencv341/build.mipsisa32r2el/install

# 3. Данные приложения
APP_NAME := $(shell grep "MENUNAME" package.conf | cut -d'=' -f2 | tr -d '"')
MAJOR := $(shell grep "APPMAJORVERSION" package.conf | cut -d'=' -f2 | tr -d '"')
MINOR := $(shell grep "APPMINORVERSION" package.conf | cut -d'=' -f2 | tr -d '"')
MICRO := $(shell grep "APPMICROVERSION" package.conf | cut -d'=' -f2 | tr -d '"')
APP_VER := $(MAJOR).$(MINOR).$(MICRO)
APP_DEFS = -DAPP_NAME_STR=\"$(APP_NAME)\" -DAPP_VER_STR=\"$(APP_VER)\"

# 4. ФОРМИРОВАНИЕ ФЛАГОВ (Критически важный момент)
# Мы добавляем ваши пути ПЕРЕД системными путями SDK
COMMON_FLAGS = $(APP_DEFS) -Wall -Wextra -Werror -pedantic \
               -I$(LIBEVENT_DIR)/include \
               -I$(CJSON_DIR) \
			   -I$(OPENCV_DIR)/include

# Используем CFLAGS/CXXFLAGS, которые уже частично заполнены правилами Axis
# Фильтруем конфликтующие стандарты, если они прилетели из SDK
CXXFLAGS := $(filter-out -std=c89 -std=c90 -ansi, $(CXXFLAGS))
CXXFLAGS += $(COMMON_FLAGS) -std=c++11 -msoft-float -march=mips32r2 -EL
CFLAGS   += $(COMMON_FLAGS) -std=c89 -msoft-float -march=mips32r2 -EL

# 5. ЛИНКОВКА
# Чтобы libevent нашелся, его путь должен быть первым в LDFLAGS
LDFLAGS := -L$(LIBEVENT_DIR)/lib -L$(OPENCV_DIR)/lib -L$(OPENCV_DIR)/share/OpenCV/3rdparty/lib $(LDFLAGS)
# Пути к SDK из вашего Makefile (на всякий случай, если common.mak их не прописал)
LDFLAGS += -L$(prefix)/lib -L$(prefix)/usr/lib

# Добавляем rpath-link для разрешения зависимостей libcapture (glib, dbus и т.д.)
LDFLAGS += -Wl,-rpath-link,$(prefix)/usr/lib:$(prefix)/lib

# Минимальный набор для opencv2/core/core.hpp
# Порядок: сначала OpenCV, потом зависимости (zlib), потом системные либы
OPENCV_MIN_LIBS = -lopencv_core -lzlib

LDLIBS = -levent $(OPENCV_MIN_LIBS) -lpthread -lcapture -lrt -lm -ldl $(LIBS)

TARGET = tixerver

CPPSRC = main.cpp camera_info.cpp network_server.cpp math_engine.cpp command_processor.cpp capturehandler.cpp rowMatClass.cpp traffcounter.cpp config_parser.cpp
CSRC = $(CJSON_DIR)/cJSON.c

OBJECTS = main.o camera_info.o network_server.o math_engine.o command_processor.o capturehandler.o rowMatClass.o traffcounter.o config_parser.o cJSON.o

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
