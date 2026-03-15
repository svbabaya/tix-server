## How to build static libevent for Axis

1. Клонировать из репозитория или скачать архив **.tar.gz** нужной версии в отдельную директорию проекта, например в /libs или отдельно от проекта в домашнюю директорию /home/{username}/libs

2. Разархивировать: '$ tar -xvzf libevent-2.1.12-stable.tar.gz'

3. Зайти в директорию библиотеки

4. Создать конфигурацию сборки (пример для mips): '$ ./configure --host=mipsisa32r2el-axis-linux-gnu --prefix=./build --disable-shared --enable-static --disable-openssl CC=mipsisa32r2el-axis-linux-gnu-gcc'

    Возможна ошибка:
    bash: ./configure: Permission denied
    Что значит отсутствие прав на исполнение у ./configure
    Предоставляем права на исполнение: 
    '''
    $ chmod +x configure
    '''
    Снова создаем конфигурацию

    Возможна ошибка:
    configure: error: expected an absolute directory name for --prefix: ./build
    Что значит необходимость указать абсолютный путь для папки сборкию ./buid
    '''
    $ ./configure --host=mipsisa32r2el-axis-linux-gnu --prefix=/Users/sergebabayan/vscode-workspace/tix-server/tixerver/libs/build --disable-shared --enable-static --disable-openssl CC=mipsisa32r2el-axis-linux-gnu-gcc
    '''

5. Выполнить '$ make'
    Эта команда скомпилирует исходный код под архитектуру процессора mips

    Возможна ошибка:
    CDPATH="${ZSH_VERSION+.}:" && cd . && /bin/bash /Users/sergebabayan/vscode-workspace/tix-server/tixerver/libs/libevent-2.1.12-stable/build-aux/missing aclocal-1.16 -I m4
    /Users/sergebabayan/vscode-workspace/tix-server/tixerver/libs/libevent-2.1.12-stable/build-aux/missing: line 81: aclocal-1.16: command not found
    WARNING: 'aclocal-1.16' is missing on your system.
         You should only need it if you modified 'acinclude.m4' or
         'configure.ac' or m4 files included by 'configure.ac'.
         The 'aclocal' program is part of the GNU Automake package:
         <https://www.gnu.org/software/automake>
         It also requires GNU Autoconf, GNU m4 and Perl in order to run:
         <https://www.gnu.org/software/autoconf>
         <https://www.gnu.org/software/m4/>
         <https://www.perl.org/>
    make: *** [Makefile:1421: aclocal.m4] Error 127

    Такая ошибка может возникнуть если в директории с библиотекой уже создавали какую-то конфигурацию и запускали make.
    Чтобы исправить, выдаем права на запуск всем скриптам и configure, обновляем временные метки с помощью touch:
    '''
    $ find . -type f -name "*.sh" -exec chmod +x {} +
    $ chmod +x configure
    $ touch configure.ac aclocal.m4 configure Makefile.am Makefile.in config.h.in
    После этого заново делаем то, что указано в пункте 4 и запускаем make
    '''

6. Выполнить '$ make install'
    Эта команда сформирует в директории /build структуру библиотеки

    Возможна ошибка:
    make[1]: Entering directory '/Users/sergebabayan/vscode-workspace/tix-server/tixerver/libs/libevent-2.1.12-stable'
    make[2]: Entering directory '/Users/sergebabayan/vscode-workspace/tix-server/tixerver/libs/libevent-2.1.12-stable'
    build-aux/install-sh -c -d '/Users/sergebabayan/vscode-workspace/tix-server/tixerver/libs/build/bin'
    /bin/bash: line 4: build-aux/install-sh: Permission denied
    make[2]: *** [Makefile:1718: install-dist_binSCRIPTS] Error 1
    make[2]: Leaving directory '/Users/sergebabayan/vscode-workspace/tix-server/tixerver/libs/libevent-2.1.12-stable'
    make[1]: *** [Makefile:2807: install-am] Error 2
    make[1]: Leaving directory '/Users/sergebabayan/vscode-workspace/tix-server/tixerver/libs/libevent-2.1.12-stable'
    make: *** [Makefile:2801: install] Error 2
    Это означает отсутствие прав на исполнение для install-sh

    Исправляем:
    '''
    $ chmod +x build-aux/install-sh
    Снова запускаем инсталяцию:
    $ make install
    '''
Если библиотека собрана в директории build/
Библиотеки: /Users/sergebabayan/vscode-workspace/tix-server/tixerver/libs/build/lib/
Хедеры: /Users/sergebabayan/vscode-workspace/tix-server/tixerver/libs/build/include/

В makefile проекта необходимо указать путь к директориям **/include** и **/lib**
