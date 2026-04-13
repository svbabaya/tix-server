## How to install and use Axis SDK in Virtual Mashine
Установка Axis SDK в WSL (Windows) или LiMa (Mac OS). Подходит для установки в различных сборках Linux и Docker контейнерах.

### 1. Установка тулчейна SDK Axis
Axis SDK v2 упакован в архив AXIS_Embedded_Development_SDK_2_0_3.tar.gz, скопируем его в домашнюю директорию виртуальной машины "~" (в LiMa это /home/{your_user_name}).
- Распакуем архив:
```
tar -xvzf AXIS_Embedded_Development_SDK_2_0_3.tar.gz
```
- В результате в той же директории появится файл `install-sdk-2_0_3.bin`, установим для него permissions на исполнение:
```
chmod +x install-sdk-2_0_3.bin
``` 
- Запустим .bin
```
./install-sdk-2_0_3.bin
```
В результате в домашней директории появится директория **axis**
- Заходим в axis/emb-app-sdk_2_0_3/tools/compilers, здесь три директории: arm, armhf, mips. Внутри каждой находится файл .deb, то есть дистрибутив тулчейна под определенную архитектуру камеры. Поочередно заходим в каждую из этих директорий и запускаем установку дистрибутивов. Пример для mips:
```
sudo dpkg -i comptools-arm-r21_1.21-1_amd64.deb
```
Тулчейны по умолчанию устанавливаются в системную директорию /usr/local:
```
/usr/local/mipsisa32r2el/r23/bin
/usr/local/arm/r21/bin
/usr/local/armhf/r27/bin
```
В этих директориях кросскомпиляторы и вспомогательные утилиты для сборки и отладки исполняемого файла под соответствующую архитектуру.

### 2. Настройка окружения для сборки
- Заходим в ~/axis/emb-app-sdk_2_0_3 и запускаем скрипт **init_env**, который настраивает окружение для работы тулчейнов:
```
source init_env
```
Если все в порядке, вы увидите такую информацию:
```
Using compiler "/usr/local/mipsisa32r2el/r23/bin/mipsisa32r2el-axis-linux-gnu-gcc" (revision "23").

Found arm-axis-linux-gnueabi-gcc r21
Using compiler "/usr/local/arm/r21/bin/arm-axis-linux-gnueabi-gcc" (revision "21").

Found arm-axis-linux-gnueabihf-gcc r27
Using compiler "/usr/local/armhf/r27/bin/arm-axis-linux-gnueabihf-gcc" (revision "27").

Prepending "/home/sergebabayan.linux/axis/emb-app-sdk_2_0_3/tools/bin" to PATH.
Prepending "/home/sergebabayan.linux/axis/emb-app-sdk_2_0_3/tools/scripts" to PATH.

Setting PKG_CONFIG_PATH.

Detected x86_64 host
'host/host-x86_64' -> 'host-x86_64'
```
Если вы распаковали SDK не в домашней, а где-нибудь в системной директории, например в /usr/local и при этом init_env не запускается, возможно нужно дать разрешение на запись для процессов, которые запускает init_env. Проще всего это сделать сменой владельца директории emb-app-sdk_2_0_3:
```
sudo chown -R {your_user_name}:{your_group_name} /usr/local/axis/emb-app-sdk_2_0_3/
```
Важно помнить, что каждый раз после загрузки операционной системы вам нужно открыть терминал, зайти в  `{your_home_dir_in_linux}/axis/emb-app-sdk_2_0_3/`, запустить скрипт ` source init_env` и только потом, в том же окне терминала вызывать скрипт create-package.sh, который запускает компиляцию с помощью make.

### 3. Устанавливаем make
В этом проекте для сборки под Axis используется make
Проверяем наличие:
```
make --version
```
Если нет - устанавливаем:
```
sudo apt install make
```
### 3. Настройка скриптов Axis SDK
Для компиляции и создания пакета загрузки на камеру (.eap) запускается скрипт с ключами, обозначающими архитектуру процессора камеры `$ create-package.sh {mipsisa32r2el | armv7hf}`. На основе этого ключа скрипт выбирает нужный toolchain. Обратите внимание, что хотя скрипт и toolchain расcчитаны для работы с другими архитектурами, в этом проекте все настройки сделаны только для mipsisa32r2el и armv7hf.

После компиляции скрипт создает пакет .eap (это архив tar) в который, помимо исполняемого файла, включаются файлы, указанные в package.conf. 

В этой версии проекта traffixtream используются отдельные makefile для разных типов камер: makefile.axis, makefile.dahua, makefile.hik и для того, чтобы create-package.sh мог с ними работать, а также принимать дополнительный параметр (серийный номер), необходимо сделать корректировку основного скрипта create-package.sh, вспомогательного eap-create.sh и eap-install.sh.

Скрипты находим в директории SDK Axis `{your_home_dir_in_linux}/axis/emb-app-sdk_2_0_3/tools/scripts`

В директории **sh** проекта есть исправленные версии скриптов, можете просто скопировать их в директорию scripts, а можете внести изменения самостоятельно на основе дальнейших инструкций.

- Открываем **create-package.sh** в любом текстовом редакторе, например так:
```
nano create-package.sh
```

- Меняем первую строчку скрипта (Shebang)
Было: `#!/bin/sh -e`
Стало: `#!/bin/bash -e`
Это необходимо, поскольку синтаксис [[ ]], использованный далее, не работает в sh.

- Перед строками:
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
Теперь из скрипта в makefile.axis будут передаваться два параметра: TARGET (архитектура) и SN(серийный номер, для которого предназначена сборка). Они же передаются в скрипт eap-create.sh.

- Сохраняем изменения
- Открываем **eap-create.sh** в текстовом редакторе:
```
nano eap-create.sh
```
- Перед определением функции doMakeTheTar() добавляем переменную SN_PARAM, которой присваивается 
значение второго параметра скрипта (серийный номер), а внутри функции doMakeTheTar() меняем код:
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
- Пока eap-create.sh открыт в текстовом редакторе, вносим еще одно изменение. В скрипте eap-create.sh для создания пакета .eap вызывается утилита tar, а поскольку оригинальный скрипт написан для устаревшей версии SDK, то в нем используется устаревший синтаксис tar. Если вы используете современную версию Linux, то без коррекции скорее всего увидите примерно похожую ошибку:
```
Creating Package: 'TraffiXtreamS_3_0-2_mipsisa32r2el.eap'... tar: The following options were used after non-option arguments.  These options are positional and affect only arguments that follow them.  Please, rearrange them properly.
tar: --exclude '*~' has no effect
tar: --exclude 'CVS' has no effect
tar: Exiting with failure status due to previous errors
failed
``` 
Ннаходим вызов tar в eap-create.sh и меняем положение ключей.

Было:
```
\tar czf $tarb $APPNAME $ADPPACKCFG $ADPPACKPARAMCFG \
	$POSTINSTALLSCRIPT $OTHERFILES $HTTPD_CONF_LOCAL_FILES \
	$HTTPD_MIME_LOCAL_FILES $HTMLDIR $EVENT_DECLS_DIR $LIBDIR \
	$LUAPKGFILES $HTTPCGIPATHS --exclude="*~" --exclude="CVS" --format=gnu || {
```
Стало:
```
\tar czf $tarb --exclude="*~" --exclude="CVS" --format=gnu $APPNAME $ADPPACKCFG $ADPPACKPARAMCFG \
	$POSTINSTALLSCRIPT $OTHERFILES $HTTPD_CONF_LOCAL_FILES \
	$HTTPD_MIME_LOCAL_FILES $HTMLDIR $EVENT_DECLS_DIR $LIBDIR \
	$LUAPKGFILES $HTTPCGIPATHS || {
```
- Сохраняем изменения
- Открываем **eap-install.sh** в текстовом редакторе:
```
nano eap-install.sh
```
- Комментируем код со строки 349 по 368:
```
# if [ "$APPMAJORVERSION" -a "$APPMINORVERSION" ]; then
	# 	if [ "$APPMICROVERSION" ]; then
	# 		end="-${APPMICROVERSION}_$APPTYPE.eap"
	# 	else
	# 		end="_$APPTYPE.eap"
	# 	fi
	# 	name=${myeap%_?_?$end}
	# 	version=$myeap
	# 	version=${version#$name}
	# 	version=${version#_}
	# 	version=${version%$end}

	# 	if [ "$version" != "${APPMAJORVERSION}_$APPMINORVERSION" ] && [ "$action" = install ]; then
	# 		# Typically happens when the version is changed
	# 		# and the eap file of the old version still is in
	# 		# the directory.
	# 		echo "$myeap doesn't match information in package.conf, skipping"
	# 		continue
	# 	fi
	# fi
```
Это сделано для предотвращения ошибки, когда скрипт сравнивает имя пакета с версиями из package.conf. Эта ошибка не позволит загрузить пакет на камеру из командной строки.
- Сохраняем изменения

Исправленные версии скриптов в итоге должны оказаться в директории SDK Axis `{your_home_dir_in_linux}/axis/emb-app-sdk_2_0_3/tools/scripts`
В директории **sh** проекта есть исправленные версии скриптов, можете просто скопировать их в scripts, а можете внести изменения самостоятельно как это показано выше.

Теперь для компиляции и создания пакета .eap возможны такие варианты:
```
$ create-package.sh mipsisa32r2el 00408CE36579
$ create-package.sh armv7hf free // Сборка не привязана к определенной камере
$ create-package.sh armv7hf 00408CE86871
$ create-package.sh mipsisa32r2el // по умолчанию в makefile.axis будет передан SN=free
```
В результате будут созданы пакеты:
```
TiXerver_1_0-1_mipsisa32r2el_00408CE36579.eap
TiXerver_1_0-1_armv7hf_free.eap
TiXerver_1_0-1_armv7hf_00408CE86871.eap
TiXerver_1_0-1_mipsisa32r2el_free.eap
```
Название приложения и версию скрипт берет из файла package.conf





!!!!!!  ToThink Если TARGET используется только на уровне makefile, то возможно нет необходимости на основе TARGET создавать дублирующий его ARCH

Обратите внимание, что в **makefile.axis** на основе TARGET, полученного из скрипта create-package.sh, создается макрос ARCH со значениями MIPS или ARMV7HF, которые используются для подключения специфических исходников и библиотек:
```
ifeq ($(TARGET), mipsisa32r2el-axis-linux-gnu)
    ARCH = MIPS
else ifeq ($(TARGET), armv7-axis-linux-gnueabihf)
    ARCH = ARMV7HF
endif
```
