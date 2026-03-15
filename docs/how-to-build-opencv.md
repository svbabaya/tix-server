## How to build opencv for Axis

1. Клонировать из репозитория или скачать архив .tar.gz нужной версии в домашнюю директорию **/home/{username}/libs**. Можно скачать и собрать внутри проекта, потом удалить исходники, или настроить .gitignore, чтобы не отправлять на github ненужные файлы. На старых камерах с последними версиями opencv могут быть проблемы. Для камер mips рекомендуется использовать opencv не выше 3 версии.

2. В корневой папке opencv создать файл конфигурации для cmake с учетом архитектуры камеры: **mips.toolchain.cmake**. 
Камеры mips не имеют аппаратной поддержки вычислений с плавающей точкой, поэтому при компиляции нужно включать режим *soft-float*

3. В mips.toolchain.cmake необходимо учитывать какие библиотеки нужны (статические/динамические) наличие VFP/FPU в существующей архитектуре и прочие особенности архитектуры:

'''
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR mips)

    /# 1. Путь к бинарникам (где лежит gcc)
    set(TOOLCHAIN_BIN_DIR "/usr/local/mipsisa32r2el/r23/bin")

    /# 2. Путь к sysroot (корень целевой системы)
    /# Мы берем папку, в которой лежат lib и sys-include
    set(CMAKE_FIND_ROOT_PATH "/usr/local/mipsisa32r2el/r23/mipsisa32r2el-axis-linux-gnu")

    /# Указываем компиляторы
    set(CMAKE_C_COMPILER "${TOOLCHAIN_BIN_DIR}/mipsisa32r2el-axis-linux-gnu-gcc")
    set(CMAKE_CXX_COMPILER "${TOOLCHAIN_BIN_DIR}/mipsisa32r2el-axis-linux-gnu-g++")

    /# Настройки поиска
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

    /# Добавляем sys-include в пути поиска, так как имя нестандартное
    include_directories(BEFORE SYSTEM "${CMAKE_FIND_ROOT_PATH}/sys-include")

    /# Указываем флаги компиляции для MIPS без аппаратного FPU
    set(CMAKE_C_FLAGS "-msoft-float -march=mips32r2 -EL" CACHE STRING "C flags" FORCE)
    set(CMAKE_CXX_FLAGS "-msoft-float -march=mips32r2 -EL" CACHE STRING "C++ flags" FORCE)

    /# Важно для OpenCV: принудительно отключаем использование VFP/FPU на уровне исходников
    set(ENABLE_VFP OFF CACHE BOOL "" FORCE)
    set(ENABLE_NEON OFF CACHE BOOL "" FORCE)
'''

4. В корневой папке opencv создать папку **/build** 

5. Зайти в build из терминала Linux и создать конфигурацию для сборки:

'''
    cmake -D CMAKE_TOOLCHAIN_FILE=../mips.toolchain.cmake \
        -D CMAKE_BUILD_TYPE=Release \
        -D CMAKE_INSTALL_PREFIX=./install \
        -D BUILD_SHARED_LIBS=OFF \
        -D BUILD_WITH_STATIC_CRT=ON \
        -D BUILD_opencv_apps=OFF \
        -D BUILD_EXAMPLES=OFF \
        -D BUILD_PACKAGE=OFF \
        -D BUILD_TESTS=OFF \
        -D BUILD_PERF_TESTS=OFF \
        -D CPU_BASELINE="" \
        -D CPU_DISPATCH="" \
        -D WITH_ITT=OFF \
        -D WITH_TIFF=OFF \
        -D WITH_OPENCL=OFF \
        -D WITH_CUDA=OFF \
        ..
'''

6. Запустить сборку:

    '$ make -j$(nproc)' /# Разрешить компиляцию в несколько потоков (nproc - количество ядер)
    
7. Сформировать структуру библиотеки:

    '$ make install'
    
Будет создана структура файлов и папок библиотеки среди которых в папке **install** находятся **include/** и **lib/**, пути к которым нужно прописать при подключении к приложению.
    