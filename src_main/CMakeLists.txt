add_executable(serv main.cpp)

# 3. Привязываем библиотеку к исполняемому файлу
# OpenCV_LIBS содержит все нужные модули (core, videoio, highgui и т.д.)
target_link_libraries(serv PRIVATE ${OpenCV_LIBS})

# Если нужны заголовочные файлы библиотеки (для старых версий CMake/OpenCV)
target_include_directories(serv PRIVATE ${OpenCV_INCLUDE_DIRS})
