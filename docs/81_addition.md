# Addition for 80_how_to_install_hikvision_sdk_in_vm.md

SDK загружается c портала Hikvision как docker образ, поэтому в первую очередь необходимо установить Docker Engine. Самый простой способ - установка Docker Desktop, который доступен для macOS, Windows и некоторых дистрибутивов Linux. При установке Docker Desktop вы получаете Docker Engine + GUI. В этом проекте используется другой подход, подразумевающий установку и использование Docker Engine через Docker CLI. Docker CLI не является Docker Desktop. Это отдельная программа нашей операционной системы. Docker CLI мы вызываем в терминале командами docker ps, docker run и т.д., то есть она работает как пульт управления для Docker Engine.

## Install Hikvision SDK on Mac
Непосредственно в macOS, без установки Docker Desktop, нельзя установить Docker Engine. Поэтому используем LiMa (Linux on Mac). LiMa создает и управляет виртуальными машинами Linux на macOS. По сути, это "движок", который поднимает среду Linux. А в дополнение к LiMa устанавливаем Colima, которая создает Docker Engine. Docker CLI на macOS подключается к этому Engine.
### 1. Устанавливаем LiMa + Colima + Docker CLI
```
brew install colima docker // эта команда автоматически установит lima
```
- Можем проверить наличие всех компонентов в системе:
```
limactl --version
colima --version
docker --version
```
### 2. Корректируем конфигурацию Docker Engine в Colima !!!!! Возможно лучше перенести далее
Это можно сделать позднее, но только после отключения Colima командой
```
colima stop
```
- Эта команда запускаем режим редактирования конфигурации в vim:
```
colima start --edit
```
- Находим секцию docker: {} и заменяем ее на такую структуру:
```
docker:
  insecure-registries:
    - "13.251.8.106"
```
После сохранения и выхода из редактора, Colima стартует автоматически.
### 3. Запускаем среду
```
colima start
```
По умолчанию Colima выделит 2 ядра CPU, 2 GiB RAM и 100 GiB диска, но можно указать другие значения:
```
colima start --cpu 4 --memory 8 --disk 50 // В последствии размер диска нельзя уменьшить
```
- Можем убедиться, что Docke Engine запущен:
```
docker version
```
В результате должна появится такая информация:
```
Client: Docker Engine - Community
 Version:           29.3.1
 API version:       1.53 (downgraded from 1.54)
 Go version:        go1.26.1
 Git commit:        c2be9ccfc3
 Built:             Wed Mar 25 14:22:32 2026
 OS/Arch:           darwin/amd64
 Context:           colima

Server: Docker Engine - Community
 Engine:
  Version:          29.2.1
  API version:      1.53 (minimum version 1.44)
  Go version:       go1.25.6
  Git commit:       6bc6209
  Built:            Mon Feb  2 17:17:26 2026
  OS/Arch:          linux/amd64
  Experimental:     false
 containerd:
  Version:          v2.2.1
  GitCommit:        dea7da592f5d1d2b7755e3a161be07f43fad8f75
 runc:
  Version:          1.3.4
  GitCommit:        v1.3.4-0-gd6d73eb8
 docker-init:
  Version:          0.19.0
  GitCommit:        de40ad0

```
Clien - это Docker CLI, Server - это Docker Engine

Еще один способ убедиться, что Docker Engine запущен, открыть список контейнеров:
```
docker ps
```
Должен появится пустой список контейнеров `CONTAINER ID  IMAGE  COMMAND  CREATED  STATUS  PORTS  NAMES`

- Можем проверить контекст, то есть какой именно Docker Engine активен в данный момент:
```
docker context ls
```
Результат такой, какой нам нужен, то есть активен контекст Docker Engine в Colima, так происходит сразу после команды `colima start`:
```
NAME            DESCRIPTION                               DOCKER ENDPOINT                                          ERROR
colima *        colima                                    unix:///Users/sergebabayan/.colima/default/docker.sock   
default         Current DOCKER_HOST based configuration   unix:///var/run/docker.sock                              
desktop-linux   Docker Desktop                            unix:///Users/sergebabayan/.docker/run/docker.sock   
```
- Может пригодиться: удаление контекста: `docker context rm {NAME}`, переключение контекста на Docker Engine в Colima: `docker context use colima`
### 4. Загружаем в Docker Engine Colima образ SDK Hikvision
#### 4.1 Вариант 1: Получаем образ на портале Hikvision
- Авторизуемся на портале **tpp.hikvision.com**
- Заходим в **HEOP MANAGER - HEOP Overview - Download Docker**
- Выбираем нужную камеру с помощью формы ввода и раскрывающегося списка
В окне появится информация для авторизации и скачивания docker образа:
```
DS-2CD3T46G2H-LISU/SL(4mm) # Выбранная из списка камера
Username: docker-global-prod
Password: ... // длинный буквенно-цифровой пароль
Docker Image URL: 13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240
```
- Заходим в Hikvision Docker Hub:
```
docker login 13.251.8.106
```
Указываем имя пользователя и пароль:
```
Username: docker-global-prod
Password: ...
```
При успешной авторизации появится сообщение `Login Succeeded`
Если ранее уже была успешная авторизация, то вводить Username и Password скорее всего не понадобится.

Если появилась ошибка типа: `Error response from daemon: Get "https://13.251.8.106/v2/": dial tcp 13.251.8.106:443: connect: connection refused`, возможно не была изменена конфигурация на шаге 2.
- Загружаем образ (Docker Image URL) в Docker Engine Colima:
```
docker pull 13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240
```
- После скачивания появится информация:
```
Digest: sha256:c1f43eb9f5a6625437e1774f420ef7ae6ec37491f93ef132d11636d99a54848b
Status: Downloaded newer image for 13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240
13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240
```
#### 4.2 Вариант 2: Получаем образ из архива
Этот способ может пригодиться, если образ уже был скачан ранее и заархивирован, например, так:
```
docker save 13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240 | gzip > ~/Desktop/hikvision_sdk_backup.tar.gz
```
Тогда вместо загрузки образа с портала выполняем команду:
```
$ docker load -i ~/Desktop/hikvision_sdk_backup.tar.gz
```
В итоге должно появиться сообщение:
```
Loaded image: 13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240
```
#### 4.3 Проверяем загрузки образа
- Открываем список образов:
```
docker images
```
- Видим наш образ, значит можно переходить к следующему шагу:
```
IMAGE                                                     ID             DISK USAGE   CONTENT SIZE   EXTRA
13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240   c1f43eb9f5a6       14.3GB         26.5kB   
```
### 5. Создаем контейнер из образа для работы с выбранным проектом
Нам нужно создать контейнер из образа SDK и при этом смонтировать локальную директорию (VOLUME_PATH) в /heop/workspace внутри контейнера для доступа к проекту. Проект находится на macOS и для монтирования нам понадобится полный адрес директории проекта. Допустим, это будет `/Users/sergebabayan/vscode-workspace/tix-server/tixcheck` 

- Загрузите скрипт **heop_devel_run.sh** из архива "G5 HEOP Developer Materials.zip" в корневую директорию проекта
- Добавьте разрешение на исполнение, если его нет:
```
$ sudo chmod +x heop_devel_run.sh
```
- Из командной строки macOs запустите скрипт:
```
$ sudo ./heop_devel_run.sh -r
```
- В ответ на это предложение вводим полный путь к директории проекта:
```
Please enter your volume path(user data path)[enter means uses default value /home/user/heop_devel_kit/volume]:
/Users/sergebabayan/vscode-workspace/tix-server/tixerver
```
Скрипт открывает список образов Docker Engine Colima и предлагает выбрать нужный:
```
VOLUME_PATH = /Users/sergebabayan/vscode-workspace/tix-server/tixcheck
WARNING: This output is designed for human readability. For machine-readable output, please use --format.
============================================
INFO:select heop_devel_kit image to run:
[   1] 13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240   67dff634b54f       28.3GB           14GB   U    
============================================
enter your choose:
```
В данном случае есть один образ с номером "1", набираем 1 и нажимаем Enter.

#### Внимание! Возможно последнее действие не сработает, это может быть вызвано ошибкой скрипта
Открываем heop_devel_run.sh в текстовом редакторе и заменяем код функции select_image() на:
```
function select_image() {
    # 1. Получаем список образов в чистом виде: "repository:tag"
    local raw_images=$(docker images --format "{{.Repository}}:{{.Tag}}")
    
    # 2. Формируем нумерованный список для отображения пользователю
    local display_list=$(echo "$raw_images" | awk '{print "["NR"]", $1}')
    
    info "select heop_devel_kit image to run:\n$display_list"
    read -p "enter your choose: " index
    
    # 3. Проверяем ввод
    if [[ "$index" =~ ^[0-9]+$ ]]; then
        # Выбираем строку по номеру и берем только имя образа
        TARGET_IMAGE=$(echo "$raw_images" | sed -n "${index}p")
    else
        TARGET_IMAGE="$index"
    fi

    # Если выбор оказался пустым (неверный индекс)
    if [[ -z "$TARGET_IMAGE" ]]; then
        err "Invalid selection. Please try again."
    fi
}
```
В директории sh проекта исправленная версия скрипта.

#### Альтернатива: запуск контейнера SDK без скрипта
Создаем контейнер сразу указывая директорию для монтирования и нужный образ: 
```
docker run -it --rm \
  -v /Users/sergebabayan/vscode-workspace/tix-server/tixcheck:/workspace \
  13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240 \
  /bin/bash
```
Если все в порядке, мы увидим похожее сообщение:
```
============================================
INFO:setup heop_devel_kit. IMAGE ID:
13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240
============================================
import /etc/heop_devel_kit.rc/001_toolchain.rc
import /etc/heop_devel_kit.rc/academy.rc
import /etc/heop_devel_kit.rc/app.rc
import /etc/heop_devel_kit.rc/dsp.rc
```
И окажемся в контейнере SDK, в директории /heop/workspace, в которой смонтирована директория нашего проекта:
```
root@36186835030a:/heop/workspace# 
```
Если выполнить команду ls, то мы увидим содержимое директории tixcheck и останется только запустить сборку:
```
root@36186835030a:/heop/workspace# make 
```

#### Как перемонтировать директорию проекта
Если необходимо собрать другой проект, делаем следующее...



### Windows


### Linux
Установка терминальной версии Desktop Engine