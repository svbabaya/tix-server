## How to install Axis SDK in Virtual Mashine

Установка Axis SDK в WSL (Windows) или LiMa (Mac OS). Подходит для установки в различных сборках Linux.

1. Установка виртуальной машины



20. Для запуска компиляции с помощью make и создания пакета загрузки на камеру (.eap) используется
скрипт с ключами, обозначающими архитектуру процессора камеры ($ create-package.sh {mipsisa32r2el | armv7hf}). На основе этого ключа скрипт выбирает нужный toolchain. Обратите внимание, что хотя скрипт и toolchain расчитаны для работы с другими архитектурами, этом проекте все настройки сделаны только для mipsisa32r2el и armv7hf.

После компиляции скрипт создает пакет .eap (это архив tar) в который помимо исполняемого файла включаются файлы,
указанные в package.conf. 

В этой версии проекта traffixtream используются отдельные makefile для разных типов камер: makefile.axis, makefile.dahua, makefile.hik и для того, чтобы create-package.sh мог с ними работать, а также принимать дополнительный параметр (серийный номер), необходимо сделать корректировку основного скрипта create-package.sh, а также вспомогательного.

Скрипты находим в директории SDK Axis(это путь по умолчанию):
{your_home_dir_in_virtual_mashine}/axis/emb-app-sdk_2_0_3/tools/scripts

В директории **sh** проекта есть исправленные версии обоих скриптов, можно просто скопировать их в директорию scripts, а можно внести изменения самостоятельно на основе дальнейших инструкций.

- Открываем **create-package.sh** в текстовом редакторе:
`$ nano create-package.sh`

- Меняем первую строчку скрипта (Shebang)
Было: `#!/bin/sh -e`
Стало: `#!/bin/bash -e`
Это необходимо, поскольку синтаксис [[ ]], использованный далее, не работает в sh.

- Перед строками
```
# The chip names are allowed as targets for backward compatability.
for target in $TARGETS ; do
```
вставляем блок, который считывает дополнительный аргумент (серийный номер)
и производит валидацию по правилу:  второй аргумент это либо free, 
либо 12-значное 16-ричное число без пробелов. Если второго аргумента нет,
то по умолчанию это free:
```
### New code
# Обработка и валидация SN перед циклом
RAW_SN="${2:-free}" # Если пусто — free
SN=$(echo "$RAW_SN" | tr '[:lower:]' '[:upper:]') # В верхний регистр
# Валидация: только "FREE" или 12 символов HEX
if [[ "$SN" != "FREE" ]] && [[ ! "$SN" =~ ^[0-9A-F]{12}$ ]]; then
    printf "Error: Invalid Serial Number '$2'\n"
    printf "SN must be 'free' or a 12-digit hex string (e.g. 00408CE86871)\n"
    help
    exit 1
fi
# Если валидация пройдена, оставляем маленькими буквами только слово free
[ "$SN" = "FREE" ] && SN="free"
### end New code
```

- Далее находим блок:
```
\printf "make $TARGET"
        \make $TARGET
        \printf "make"
        \make || {
                croak "make failed. Please fix above errors, before you can create a package"
                exit 1
        }
        \command -v eap-create.sh > /dev/null 2>&1 && \eap-create.sh $TARGET || `dirname $0`/eap-create.sh $TARGET
```
И меняем его на:
```
### New version
\printf "make -f makefile.axis TARGET=$TARGET SN=$SN\n"
        \make -f makefile.axis TARGET=$TARGET SN=$SN || {
                croak "make failed. Please fix above errors, before you can create a package"
                exit 1
        }
\command -v eap-create.sh > /dev/null 2>&1 && \eap-create.sh $TARGET "$SN" || `dirname $0`/eap-create.sh $TARGET "$SN"
### end New version
```
Теперь в makefile.axis будут передаваться два параметра: TARGET (архитектура) и SN(серийный номер).
Они же передаются в скрипт eap-create.sh

- Сохраняем изменения
- Открываем **eap-create.sh** в текстовом редакторе:
`$ nano eap-create.sh`
- Перед определением функции doMakeTheTar() добавляем переменную SN_PARAM, которой присваивается 
значение второго параметра скрипта (серийный номер), а внутри функции doMakeTheTar() меняем код 
в выделенной области:
```
SN_PARAM="$2"
doMakeTheTar() {
	local tarb

	# Create the name of the file.
	tarb=$(echo "$PACKAGENAME" | \sed 's/ /_/g')
	if [ "$APPMAJORVERSION" ] && [ "$APPMINORVERSION" ]; then
		tarb=${tarb}_${APPMAJORVERSION}_$APPMINORVERSION
		[ -z "$APPMICROVERSION" ] || tarb=${tarb}-$APPMICROVERSION
	fi

### Old version
	# if [ -z $APPTYPE ]; then
	# 	 tarb=$tarb.eap
	# else
	#	 tarb=${tarb}_$APPTYPE.eap
	# fi
### end Old version

### New version
	# Добавляем архитектуру
        if [ -n "$APPTYPE" ]; then
                tarb="${tarb}_${APPTYPE}"
        fi
        # Добавляем серийный номер В САМОМ КОНЦЕ
        if [ -n "$SN_PARAM" ]; then
                tarb="${tarb}_${SN_PARAM}"
        fi
        # Добавляем расширение
        tarb="${tarb}.eap"
### end New version

	LUAPKGFILES=
        # ... остальной код функции
}
```
- Сохраняем изменения

Теперь использовать скрипт для компиляции и создания пакета .eap нужно так:
```
$ create-package.sh mipsisa32r2el 00408CE36579
$ create-package.sh armv7hf free
$ create-package.sh armv7hf 00408CE86871
$ create-package.sh mipsisa32r2el // по умолчанию в makefile.axis будет передан SN=free
```
В результате будут создаваться пакеты похожего формата:
```
TiXerver_1_0-1_mipsisa32r2el_00408CE86871.eap
TiXerver_1_0-1_armv7hf_free.eap
```
Название приложения и версию скрипт берет из файла package.conf

Обратите внимание, что в **makefile.axis** на основе TARGET, полученного из скрипта create-package.sh
создается макрос ARCH со значениями MIPS или ARMV7HF, которые используются для подключения специфических 
исходников и библиотек:
```
ifeq ($(TARGET), mipsisa32r2el-axis-linux-gnu)
    ARCH = MIPS
else ifeq ($(TARGET), armv7-axis-linux-gnueabihf)
    ARCH = ARMV7HF
endif
```
