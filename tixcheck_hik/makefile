# Компилятор и флаги
# CXX = aarch64-linux-gnu-g++
CXXFLAGS = -Wall -Wextra -O2
LDFLAGS = 

# Целевой файл
TARGET = tixcheck
SOURCES = main.cpp
OBJECTS = $(SOURCES:.cpp=.o)

# Правило по умолчанию
all: $(TARGET)

# Линковка
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Компиляция
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Очистка
clean:
	rm -f $(OBJECTS) $(TARGET)

# Проверка типа файла (опционально)
check: $(TARGET)
	file $(TARGET)

.PHONY: all clean check