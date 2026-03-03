# Makefile for building tixerver with different camera types
# How to use: $ make clean release CAMERA_TYPE={AXIS|STUB} or $ make clean debug CAMERA_TYPE={AXIS|STUB}
# By default $ make == $ make clean release CAMERA_TYPE=AXIS

# 0. Настройки по умолчанию
CAMERA_TYPE ?= AXIS
# Чтобы 'make' без аргументов делал 'clean release'
.DEFAULT_GOAL := release_default

export AXIS_TOP_DIR = /home/babay/axis_sdk/axis/emb-app-sdk_2_0_3
AXIS_USABLE_LIBS = UCLIBC GLIBC

# 1. Подключаем правила Axis
include $(AXIS_TOP_DIR)/tools/build/rules/common.mak

# 2. Локальные пути
LIBEVENT_DIR = $(CURDIR)/libs/build/libevent_mipsisa32r2el
CJSON_DIR = $(CURDIR)/libs/cjson
OPENCV_DIR = /mnt/c/viat/libs/opencv341/build.mipsisa32r2el/install

# Поддиректории для VPATH и Includes
SUBDIRS = . src/common src/capture src/network src/maths src/maths/legacy
VPATH = $(SUBDIRS)
INC_FLAGS = $(addprefix -I, $(SUBDIRS))

# 3. Данные приложения
APP_NAME := $(shell grep "APPNAME" package.conf | cut -d'=' -f2 | tr -d '"')
MAJOR := $(shell grep "APPMAJORVERSION" package.conf | cut -d'=' -f2 | tr -d '"')
MINOR := $(shell grep "APPMINORVERSION" package.conf | cut -d'=' -f2 | tr -d '"')
MICRO := $(shell grep "APPMICROVERSION" package.conf | cut -d'=' -f2 | tr -d '"')
APP_VER := $(MAJOR).$(MINOR).$(MICRO)

# Записать данные о названии приложения и версии в исходный код в виде макросов APP_NAME, APP_VER
APP_DEFS = -DAPP_NAME_STR=\"$(APP_NAME)\" -DAPP_VER_STR=\"$(APP_VER)\"

# Список общих объектов
COMMON_OBJECTS = main.o camera_info.o network_server.o math_engine.o \
                 command_processor.o rowMatClass.o traffcounter.o \
                 parser_config.o capture_factory.o cJSON.o
                 # old_traffcounter.o old_datastructs.o old_traffsensor.o old_traffzone.o old_utils_cam.o old_utils_traff.o

# Логика выбора реализации камеры
ifeq ($(CAMERA_TYPE), AXIS)
    CAMERA_OBJ = capture_axis.o
    APP_DEFS += -DCAMERA_AXIS
    SPECIFIC_LDLIBS = -lcapture
else ifeq ($(CAMERA_TYPE), STUB)
    CAMERA_OBJ = capture_stub.o
    APP_DEFS += -DCAMERA_STUB
    SPECIFIC_LDLIBS = 
else
    $(error Unknown CAMERA_TYPE: $(CAMERA_TYPE))
endif

OBJECTS = $(COMMON_OBJECTS) $(CAMERA_OBJ)

# 4. ФОРМИРОВАНИЕ ФЛАГОВ (Теперь APP_DEFS содержит нужный макрос)
COMMON_FLAGS = $(APP_DEFS) $(INC_FLAGS) -Wall -Wextra -Werror -pedantic \
               -I$(LIBEVENT_DIR)/include \
               -I$(CJSON_DIR) \
               -I$(OPENCV_DIR)/include

CXXFLAGS := $(filter-out -std=c89 -std=c90 -ansi, $(CXXFLAGS))
CXXFLAGS += $(COMMON_FLAGS) -std=c++11 -msoft-float -march=mips32r2 -EL
CFLAGS   += $(COMMON_FLAGS) -std=c89 -msoft-float -march=mips32r2 -EL

CXXFLAGS += -Isrc/maths/legacy

# 5. ЛИНКОВКА
# Чтобы libevent нашелся, его путь должен быть первым в LDFLAGS
LDFLAGS := -L$(LIBEVENT_DIR)/lib -L$(OPENCV_DIR)/lib -L$(OPENCV_DIR)/share/OpenCV/3rdparty/lib $(LDFLAGS)
# Пути к SDK из Makefile (на всякий случай, если common.mak их не прописал)
LDFLAGS += -L$(prefix)/lib -L$(prefix)/usr/lib
# Добавляем rpath-link для разрешения зависимостей libcapture (glib, dbus и т.д.)
LDFLAGS += -Wl,-rpath-link,$(prefix)/usr/lib:$(prefix)/lib

# Минимальный набор для opencv2/core/core.hpp
# Порядок: сначала OpenCV, потом зависимости (zlib), потом системные либы
OPENCV_MIN_LIBS = -lopencv_core -lzlib
LDLIBS = -levent $(OPENCV_MIN_LIBS) -lpthread $(SPECIFIC_LDLIBS) -lrt -lm -ldl $(LIBS)

TARGET = tixerver

# --- ЦЕЛИ СБОРКИ ---

# По умолчанию делает полную пересборку в release
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
