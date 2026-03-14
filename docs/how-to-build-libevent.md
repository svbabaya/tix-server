## How to build libevent for Axis

1. Клонировать из репозитория или скачать архив .tar.gz нужной версии в отдельную директорию проекта, например в /libs или отдельно от проекта в домашнюю директорию /home/{username}/libs

2. Разархивировать: $ tar -xvzf libevent-2.1.12-stable.tar.gz

3. Зайти в директорию библиотеки

4. Создать сонфигурацию сборки (пример для mips): $ ./configure --host=mipsisa32r2el-axis-linux-gnu --prefix=./build --disable-shared --enable-static --disable-openssl CC=mipsisa32r2el-axis-linux-gnu-gcc

5. Выполнить $ make

6. Выполнить $ make install

7. В makefile проекта указать путь к директориям собранной библиотеки (обычно /include и /lib)
