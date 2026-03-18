## How to install Axis SDK in Virtual Mashine

Установка Axis SDK в WSL (Windows) или LiMa (Mac OS). Подходит для установки в различных сборках Linux.

1. Установка виртуальной машины



20. Для запуска компиляции с помощью make и создания пакета загрузки на камеру (.eap) используется
скрипт с ключами, обозначающими архитектуру процессора камеры ($ create-package.sh {mipsisa32r2el | armv7hf}). На основе этого ключа скрипт выбирает нужный toolchain.
После компиляции скрипт создает пакет .eap в который помимо исполняемого файла включаются дополнительные файлы,
указанные в package.conf. В проекте traffixtream предусмотрено использование отдельных makefile для разных типов камер: 
makefile.axis, makefile.dahua, makefile.hik и для того, чтобы create-package.sh мог работать,
необходимо сделать корректировку, поскольку скрипт будет запрашивать именно makefile.
Кроме того, нужно обеспечить возможность передачи серийного номера камеры в код перед компиляцией и сообщать коду под какую архитектуру будет производиться компиляция, поскольку от выбора архитектуры зависит подключение и отключение некоторых фрагментов кода.
Решение:
- Находим скрипт в директории SDK: {your_home_dir}/axis/emb-app-sdk_2_0_3/tools/scripts и открываем
в текстовом редакторе:
`$ nano create-package.sh`
- Находим два вызова make:
```
...
\printf "make $TARGET"
        \make $TARGET
        \printf "make"
        \make || {
                croak "make failed. Please fix above errors, before you can create a package"
                exit 1
        }
...
```
- Корректируем вызов make:
```
...
\printf "make -f makefile.axis $target TARGET=$target ${@:2}\n"
        \make -f makefile.axis $target TARGET=$target "${@:2}"
        \printf "make -f makefile.axis TARGET=$target ${@:2}\n"
        \make -f makefile.axis TARGET=$target "${@:2}" || {
                croak "make failed. Please fix above errors, before you can create a package"
                exit 1
        }
...
```
Комментарии:
Первый make вызывает специфичный таргет архитектуры, второй вызывает сборку
\-f makefile.axis: Принудительно использует makefile.axis вместо стандартного makefile.
TARGET=$target: Передает полное имя архитектуры (например, mipsisa32r2el-axis-linux-gnu) внутрь makefile.axis. 
"${@:2}": Берет все аргументы, которые вы ввели после первого (например, SN=free), и передает их в make.
- Сохраняем изменения
- Формируем ARCH на основе переданной из скрипта архитектуры. В makefile.axis должен быть такой код:
```
ifeq ($(TARGET), mipsisa32r2el-axis-linux-gnu)
    ARCH = MIPS
else ifeq ($(TARGET), armv7-axis-linux-gnueabihf)
    ARCH = ARMV7HF
endif
```
- Пример использования скрипта:
$ create-package.sh mipsisa32r2el SN=free
$ create-package.sh armv7hf SN=free
$ create-package.sh armv7hf SN=00408CE86871

Возможная проблема из-за того, что синтаксис ${@:2} (срез массива аргументов, начиная со второго) поддерживается в Bash, но недопустим в стандартном sh/dash:
`.../create-package.sh: 86: Bad substitution`
Чтобы исправить ситуацию, либо запускаем скрипт через bash: `$ bash create-package.sh armv7hf SN=free`
Либо меняем первую строчку скрипта (Shebang):
Было: `#!/bin/sh -e`
Стало: `#!/bin/bash -e`
