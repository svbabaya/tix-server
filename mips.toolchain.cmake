set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR mips)

# 1. Путь к бинарникам (где лежит gcc)
set(TOOLCHAIN_BIN_DIR "/usr/local/mipsisa32r2el/r23/bin")

# 2. Путь к sysroot (корень целевой системы)
# Папка, в которой лежат lib и sys-include
set(CMAKE_FIND_ROOT_PATH "/usr/local/mipsisa32r2el/r23/mipsisa32r2el-axis-linux-gnu")

# 3. Указываем компиляторы
set(CMAKE_C_COMPILER "${TOOLCHAIN_BIN_DIR}/mipsisa32r2el-axis-linux-gnu-gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_BIN_DIR}/mipsisa32r2el-axis-linux-gnu-g++")

# 4. Настройки поиска
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# 5. Добавляем sys-include в пути поиска, так как имя нестандартное
include_directories(BEFORE SYSTEM "${CMAKE_FIND_ROOT_PATH}/sys-include")

# Указываем флаги компиляции для MIPS без аппаратного FPU
set(CMAKE_C_FLAGS "-msoft-float -march=mips32r2 -EL" CACHE STRING "C flags" FORCE)
set(CMAKE_CXX_FLAGS "-msoft-float -march=mips32r2 -EL" CACHE STRING "C++ flags" FORCE)

# Важно для OpenCV: принудительно отключаем использование VFP/FPU на уровне исходников
set(ENABLE_VFP OFF CACHE BOOL "" FORCE)
set(ENABLE_NEON OFF CACHE BOOL "" FORCE)
