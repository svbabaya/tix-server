## How to install Axis SDK in Virtual Mashine

Установка Axis SDK в WSL (Windows) или LiMa (Mac OS). Подходит для установки в различных сборках Linux.

1. Установка виртуальной машины



20. Для запуска компиляции с помощью make и создания пакета загрузки на камеру (.eap) используется
скрипт с ключами, обозначающими архитектуру процессора камеры ($ create-package.sh {mipsisa32r2el | armv7hf}). На основе этого ключа скрипт выбирает нужный toolchain.
После компиляции скрипт создает пакет .eap в который помимо исполняемого файла включаются дополнительные файлы,
указанные в package.conf. В новой версии проекта traffixtream предусмотрено использование отдельных makefile для разных типов камер: makefile.axis, makefile.dahua, makefile.hik и для того, чтобы create-package.sh мог работать, необходимо сделать его корректировку, поскольку скрипт по умолчанию использует именно makefile.
Кроме того, нужно обеспечить возможность передачи серийного номера камеры в код перед компиляцией и сообщать коду под какую архитектуру будет производиться компиляция, поскольку от выбора архитектуры зависит подключение и отключение некоторых фрагментов кода.
Решение:

- Находим **create-package.sh** в директории SDK: {your_home_dir}/axis/emb-app-sdk_2_0_3/tools/scripts и открываем в текстовом редакторе:
`$ nano create-package.sh`

- Перед строкой `for target in $TARGETS ; do...` вставляем блок, который считывает дополнительный аргумент
и производит валидацию по правилу: второй аргумент это либо free, либо 12-значное 16-ричное число без пробелов. Если второго аргумента нет, то по умолчанию это free.
```
# Обработка и валидация SN перед циклом
RAW_SN="${2:-free}"                  # Если пусто — free
RAW_SN="${RAW_SN#*=}"                # Убираем "SN=", если оно было введено
SN=$(echo "$RAW_SN" | tr 'a-f' 'A-F') # В верхний регистр

# Валидация: только "FREE" или 12 символов HEX
if [[ "$SN" != "FREE" ]] && [[ ! "$SN" =~ ^[0-9A-F]{12}$ ]]; then
    printf "Error: Invalid Serial Number '$2'\n"
    printf "SN must be 'free' or a 12-digit hex string (e.g. 00408CE86871)\n"
    help
    exit 1
fi

# Если валидация пройдена, оставляем маленькими буквами только слово free 
[ "$SN" = "FREE" ] && SN="free"
```

- Далее по коду находим два вызова make:
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

- Корректируем вызовы make и вывод информации printf:
```
...     
\printf "make -f makefile.axis $target TARGET=$target SN=$SN\n"
        \make -f makefile.axis $target TARGET=$target SN=$SN
        \printf "make -f makefile.axis TARGET=$target SN=$SN\n"
        \make -f makefile.axis TARGET=$target SN=$SN || {
                croak "make failed. Please fix above errors, before you can create a package"
                exit 1
        }
  
...
```
Комментарии:
Первый make вызывает специфичный таргет архитектуры, второй вызывает сборку.
\-f makefile.axis: Это позволяет использовать makefile.axis вместо стандартного makefile
TARGET=$target: Передает полное имя архитектуры (например, mipsisa32r2el-axis-linux-gnu) внутрь makefile.axis 
SN=$SN: Передает в makefile.axis значение SN
- Сохраняем изменения
- Открывает **makefile.axis** и формируем ARCH на основе переданной из скрипта архитектуры:
```
ifeq ($(TARGET), mipsisa32r2el-axis-linux-gnu)
    ARCH = MIPS
else ifeq ($(TARGET), armv7-axis-linux-gnueabihf)
    ARCH = ARMV7HF
endif
```

- Пример использования скрипта:
$ create-package.sh mipsisa32r2el free
$ create-package.sh armv7hf free
$ create-package.sh armv7hf 00408CE86871
$ create-package.sh mipsisa32r2el // по умолчанию в makefile.axis будет передан SN=free

- При запуске create-package.sh возможна ошибка `Bad substitution` из-за того, что синтаксис [[ ]] и ${@:2} поддерживается в Bash, но недопустим в стандартном sh/dash. Чтобы исправить ситуацию либо запускаем скрипт через bash: `$ bash create-package.sh armv7hf free`
Либо меняем первую строчку скрипта (Shebang):
Было: `#!/bin/sh -e`
Стало: `#!/bin/bash -e`

- Рекомендуется указывать серийный номер камеры в имени eap-пакета, для этого в **create-package.sh** добавляем еще один аргумент при вызове скрипта **eap-create.sh**, а именно
1.1 Меняем строку:
`\command -v eap-create.sh > /dev/null 2>&1 && \eap-create.sh $TARGET || `dirname $0`/eap-create.sh $TARGET`
на
`\command -v eap-create.sh > /dev/null 2>&1 && \eap-create.sh $TARGET "$SN" || `dirname $0`/eap-create.sh $TARGET "$SN"`

1.2 Открываем **eap-create.sh**

1.3 Перед определением функции doMakeTheTar() добавляем переменную SN_PARAM,
а внутри функции doMakeTheTar() меняем код:
```
SN_PARAM="$2"
doMakeTheTar() {
        local tarb

        # 1. Формируем базу: Имя + Версия
        tarb=$(echo "$PACKAGENAME" | \sed 's/ /_/g')
        if [ "$APPMAJORVERSION" ] && [ "$APPMINORVERSION" ]; then
                tarb=${tarb}_${APPMAJORVERSION}_$APPMINORVERSION
                [ -z "$APPMICROVERSION" ] || tarb=${tarb}-$APPMICROVERSION
        fi

        # 2. Добавляем архитектуру (если есть)
        if [ -n "$APPTYPE" ]; then
                tarb="${tarb}_${APPTYPE}"
        fi

        # 3. Добавляем серийный номер В САМОМ КОНЦЕ
        if [ -n "$SN_PARAM" ]; then
                tarb="${tarb}_${SN_PARAM}"
        fi

        # 4. Добавляем расширение
        tarb="${tarb}.eap"

        LUAPKGFILES=
        # ... остальной код функции
}
```
Теперь пакет будет выглядеть так:
TiXerver_1_0-1_mipsisa32r2el_00408CE86871.eap
TiXerver_1_0-1_armv7hf_free.eap
Название приложения и версию скрипт берет из файла package.conf
