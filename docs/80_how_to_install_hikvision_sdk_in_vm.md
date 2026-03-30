# How to install and use Hikvision SDK on MacOs, Windows, Linux

SDK загружается c портала Hikvision как docker образ, поэтому в первую очередь необходимо установить Docker Engine. Самый простой способ - установка Docker Desktop, который доступен для macOS, Windows и некоторых дистрибутивов Linux. При установке Docker Desktop вы получаете Docker Engine + GUI. В этом проекте используется другой подход, подразумевающий установку и использование Docker Engine через Docker CLI. Docker CLI не является Docker Desktop. Это отдельная программа нашей операционной системы. Docker CLI мы вызываем в терминале командами docker ps, docker run и т.д., то есть она работает как пульт управления для Docker Engine.

## Установка Hikvision SDK на macOS
Напрямую, без установки Docker Desktop, установить на macOS Docker Engine нельзя. В этом проекте все действия удобнее выполнять в CLI, поэтому было принято решение вместо Docker Desktop использовать Lima + Сolima. Lima (Linux on Mac) создает и управляет виртуальными машинами Linux на macOS. Colima (Containers on Lima), дополнение к Lima, которое создает Docker Engine для запуска docker контейнеров. Docker CLI - командная оболочка, которую мы установим на macOS для управления контейнерами. Docker CLI будет работать с Docker Engine из Colima.
### 1. Устанавливаем Lima + Colima + Docker CLI
```
brew install colima docker
```
Помимо указанных, эта команда автоматически установит Lima, поскольку Lima является зависимостью (dependency) для Colima, о чем знает пакетный менеджер
- Проверяем наличие всех компонентов в системе:
```
limactl --version
colima --version
docker --version
```
### 2. Корректируем конфигурацию Docker Engine в Colima
- Пока Colima не запущена, открываем файл конфигурации:
```
colima start --edit // по умолчанию это будет vim
```
- Находим секцию docker: {} и заменяем ее на такую структуру:
```
docker:
  insecure-registries:
    - "13.251.8.106"
```
- После сохранения и выхода из редактора, Colima стартует автоматически.
- Эту корректировку можно сделать позднее, но только после остановки Colima:
```
colima stop
```
### 3. Запускаем среду
Если Colima не была запущена ранее, делаем это командой:
```
colima start
```
По умолчанию Colima выделит 2 ядра CPU, 2 GiB RAM и 100 GiB диска, но можно стартовать с другими параметрами:
```
colima start --cpu 4 --memory 8 --disk 50
```
Вы можете и дальше менять эти значения, при условии, что размер диска допустимо только увеличивать.
- Убедимься, что Docke Engine запущен:
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
Client - это Docker CLI, Server - это Docker Engine

Еще один способ убедиться, что Docker Engine запущен - открыть список контейнеров:
```
docker ps
```
Должен появится пустой список контейнеров `CONTAINER ID  IMAGE  COMMAND  CREATED  STATUS  PORTS  NAMES`

- Иногда бывает полезно проверить контекст, то есть какой именно Docker Engine активен в данный момент:
```
docker context ls
```
Результат такой, какой нам нужен, то есть активен контекст Docker Engine в Colima, обычно он становится активным сразу после команды `colima start`:
```
NAME            DESCRIPTION                               DOCKER ENDPOINT                                          ERROR
colima *        colima                                    unix:///Users/sergebabayan/.colima/default/docker.sock   
default         Current DOCKER_HOST based configuration   unix:///var/run/docker.sock                              
desktop-linux   Docker Desktop                            unix:///Users/sergebabayan/.docker/run/docker.sock   
```
- Может пригодиться: удаление контекста: `docker context rm {NAME}`, переключение контекста на Docker Engine в Colima: `docker context use colima`
### 4. Загружаем в Docker Engine Colima образ SDK Hikvision
#### 4.1 Вариант 1: С портала Hikvision
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
#### 4.2 Вариант 2: Из архива
Этот способ пригодится, если образ уже был скачан ранее и заархивирован, например, так:
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
#### 4.3 Проверяем загрузку образа
- Открываем список образов:
```
docker images
```
- Видим наш образ:
```
IMAGE                                                     ID             DISK USAGE   CONTENT SIZE   EXTRA
13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240   c1f43eb9f5a6       14.3GB         26.5kB   
```
Значит можно переходить к следующему шагу.
### 5. Запускаем контейнер из образа
Теперь нам нужно создать контейнер из полученного образа SDK и смонтировать локальную директорию (VOLUME_PATH) в /heop/workspace внутри контейнера, чтобы получить доступ к корневой директории решения, находящегося в macOS. Для монтирования нам понадобится полный адрес корневой директории: `/Users/sergebabayan/vscode-workspace/tix-server/tixcheck` 

- Согласно руководству Hikvision, для дальнейшей работы нужно скопировать скрипт `heop_devel_run.sh` из архива "G5 HEOP Developer Materials.zip" в корневую директорию решения. С помощью него производится настройка среды компиляции. Но поскольку скрипт пришлось немного откорректировать, рекомендуется просто взять исправленный вариант из директории **sh** этого проекта. Теперь скрипт называется `heop_devel_run_fixed.sh`, скопируйте его в корневую директорию нужного решения.
- Добавляем скрипту разрешение на исполнение, если его нет:
```
$ sudo chmod +x heop_devel_run_fixed.sh
```
- Из командной строки macOs запускаем:
```
$ sudo ./heop_devel_run_fixed.sh -r
```
- Вводим полный путь к директории решения:
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
[   1] 13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240  67dff634b54f  28.3GB  14GB  U
============================================
enter your choose:
```
- В данном случае есть один образ с номером "1", набираем 1 и нажимаем Enter.
Если все в порядке, появится сообщение, означающее, что контейнер с SDK запущен и можно пльзоваться командной оболочкой операционной системы контейнера:
```
============================================
INFO:setup heop_devel_kit. IMAGE ID:
13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240
============================================
import /etc/heop_devel_kit.rc/001_toolchain.rc
import /etc/heop_devel_kit.rc/academy.rc
import /etc/heop_devel_kit.rc/app.rc
import /etc/heop_devel_kit.rc/dsp.rc
root@36186835030a:/heop/workspace# 
```
В директории /heop/workspace смонтирована директория нашего решения и если выполнить команду ls, то мы увидим содержимое директории tixcheck. Теперь можно запускать компиляцию:
```
root@36186835030a:/heop/workspace# make 
```
Выходим из контейнера обратно в macOS:
```
root@36186835030a:/heop/workspace# exit
```
#### Как перемонтировать директорию проекта
Для работы с другим проектом необходимо в /heop/workspace смонтировать другую директорию из macOS. При повторном запуске скрипта `./heop_devel_run_fixed.sh -r` сначала вы увидите какая директория уже смонтирована и вопрос о ее замене:
```
Found existing VOLUME_PATH: /Users/sergebabayan/vscode-workspace/tix-server/tixcheck
Do you want to use this path? [Y/n]:
```
Если директорию нужно сменить, отвечаем "n" и появляется предложение указать другую:
```
Please enter your volume path [default: /Users/sergebabayan/heop_devel_kit/volume]:
```
Указываем новую: `/Users/sergebabayan/vscode-workspace/tix-server/tierver` и видим информацию:
```
Using VOLUME_PATH: /Users/sergebabayan/vscode-workspace/tix-server/tixerver
============================================
INFO:select heop_devel_kit image to run:
[1] 13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240
============================================
enter your choose:
```
После выбора номера образа "1" происходит создание контейнера с новым монтированием и вход в операционную систему контейнера, как в предыдущем примере.

## Корректировки скрипта - превращаем heop_devel_run.sh в heop_devel_run_fixed.sh
### Исправление ошибки: При запросе номера образа некорректно парсятся введенные данные и в итоге образ не выбирается:
```
============================================
INFO:select heop_devel_kit image to run:
[   1] 13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240  67dff634b54f  28.3GB  14GB  U
============================================
enter your choose:
```
Заменяем функцию select_image() на этот код:
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
### Добавление функционала: При повторном вызове скрипта не предусмотрена смена директории для монтирования в контейнере, то есть с помощью скрипта не получится перейти к другому решению.
При первом запуске скрипта путь к рабочей директории записывается в скрытый файл .config внутри папки ~/heop_devel_kit, поэтому при последующих запусках скрипт уже не предлагает указывать директорию проекта. Эту задачу можно решить удалением .config в macOS: `rm ~/heop_devel_kit/.config`, но лучше модифицировать скрипт, а именно, заменить функцию load_config() на этот код:
```
function load_config() {
    # 1. Если конфиг есть, спрашиваем, оставить ли старый путь
    if [[ -e ${ROOT_DIR}/.config ]]; then
        source ${ROOT_DIR}/.config
        echo -e "\033[33mFound existing VOLUME_PATH: ${VOLUME_PATH}\033[0m"
        read -p "Do you want to use this path? [Y/n]: " choice
        
        # Если пользователь ввел 'n' или 'N', сбрасываем переменную
        if [[ "$choice" == "n" || "$choice" == "N" ]]; then
            unset VOLUME_PATH
        fi
    fi

    # 2. Если пути нет (первый запуск ИЛИ пользователь отказался от старого)
    if [[ -z "${VOLUME_PATH}" ]]; then
        read -p "Please enter your volume path (user data path) [default ${DEFAULT_VOLUME_PATH}]: " vol_path
        # Используем введенный путь или дефолтный, если нажали Enter
        VOLUME_PATH=${vol_path:-$DEFAULT_VOLUME_PATH}
        
        # Сохраняем в конфиг (перезаписываем)
        echo "VOLUME_PATH=${VOLUME_PATH}" > ${ROOT_DIR}/.config
        mkdir -p "${VOLUME_PATH}" || err "Create volume failed in ${VOLUME_PATH}"
    fi

    # echo "Using VOLUME_PATH: ${VOLUME_PATH}"
    chmod 777 "${VOLUME_PATH}"
}
```
Измененный скрипт под именем `heop_devel_run_fixed.sh` можно взять в директории sh этого проекта.



// ToDo
### Windows
Ситуация как в macOS - напрямую, без установки Docker Desktop, поставить на Windows Docker Engine нельзя. Будем использовать WSL2 (легковесная VM с полноценным ядром Linux). 



1 Установите WSL2 (если ещё нет):
wsl --install -d Ubuntu

2 Откройте терминал WSL и установите Colima и Docker CLI:
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
brew install colima docker

Установка вручную из последней версии на github
curl -L https://github.com/abiosoft/colima/releases/download/v0.9.0/colima-Linux-x86_64 -o colima
chmod +x colima
sudo mv colima /usr/local/bin/

3 Запустите Colima:
colima start --cpu 4 --memory 8 --disk 50

4 Проверить установку
docker run hello-world

// ToDo рассмотреть возможность размещения проекта в WSL2, может быть делать pull перед сборкой, открывать VSC прямо из WSL2? 
Если код лежит на Windows (под /mnt/c/), каждая операция с файлами проходит через границу WSL↔Windows, добавляя задержки. События изменения файлов (inotify) не работают — это ломает автоперезагрузку в инструментах разработки.

Рекомендация: с самого начала держите проект внутри файловой системы WSL. Из Windows к нему можно обращаться по пути \\wsl$\<дистрибутив>\home\<пользователь>\проект.

Когда проект лежит в WSL, вы всё равно можете редактировать его из Windows:
VS Code: code . из терминала WSL (используется Remote-WSL)
Проводник Windows: путь \\wsl$\Ubuntu\home\username\project
Любой редактор Windows может открывать файлы по этому UNC-пути


5 Разместите проект в файловой системе WSL. Это рекомендация, связанная с тем, что Linux и Windows имеют разные файловые системы.
cd ~/projects
git clone <ваш-проект>

Дополнение: Можно задействовать аппаратную виртуализацию для ускорения
Проверьте наличие /dev/kvm:
ls -l /dev/kvm
Если есть — добавьте себя в группу kvm:
sudo usermod -aG kvm $USER
выйдите из WSL и зайдите снова
Это включит аппаратную виртуализацию


Обобщение:
Для максимально близкого к вашему macOS-опыту:

-Установите WSL2 с Ubuntu
-Разместите проект в ~/projects внутри WSL
-Выберите один из вариантов:

Colima внутри WSL — если хотите сохранить знакомый инструмент
Docker Desktop с WSL2 — если нужна простота и официальная поддержка

В обоих случаях ваш SDK будет работать в контейнере, код — на "хосте" (файловая система WSL), а производительность будет отличной, потому что вы избежали кросс-граничного монтирования из Windows.


// ToDo
### Linux
Установка терминальной версии Desktop Engine в Linux
1.1 Добавляем ключ Docker
```
$ sudo mkdir -p /etc/apt/keyrings
$ curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /etc/apt/keyrings/docker.gpg
```
1.2 Добавляем репозиторий Docker
```
$ echo "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.gpg] https://download.docker.com/linux/ubuntu $(. /etc/os-release && echo "$VERSION_CODENAME") stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
```
1.3 Обновляем списки пакетов
```
$ sudo apt-get update
```
1.4 Устанавливаем зависимости и Docker
```
$ sudo apt-get install -y apt-transport-https ca-certificates curl gnupg-agent software-properties-common docker-ce docker-ce-cli containerd.io
```
В результате будет добавлен официальный репозиторий Docker, установлен Docker Engine, Docker Compose V2, настроен containerd и все необходимые компоненты.

Убедимся, что Docker работает корректно, то есть Docker daemon запущен и работает, Docker может скачивать образы из Docker Hub, создавать и запускать контейнеры, нет ошибок конфигурации или проблем с правами:
```
$ sudo docker run hello-world
```
Если все в порядке, быдет выведена похожая информация:
```
Unable to find image 'hello-world:latest' locally
latest: Pulling from library/hello-world
17eec7bbc9d7: Pull complete 
ea52d2000f90: Download complete 
Digest: sha256:854...
Status: Downloaded newer image for hello-world:latest

Hello from Docker!
This message shows that your installation appears to be working correctly.

To generate this message, Docker took the following steps:
 1. The Docker client contacted the Docker daemon.
 2. The Docker daemon pulled the "hello-world" image from the Docker Hub.
    (amd64)
 3. The Docker daemon created a new container from that image which runs the
    executable that produces the output you are currently reading.
 4. The Docker daemon streamed that output to the Docker client, which sent it
    to your terminal.

To try something more ambitious, you can run an Ubuntu container with:
 $ docker run -it ubuntu bash

Share images, automate workflows, and more with a free Docker ID:
 https://hub.docker.com/

For more examples and ideas, visit:
 https://docs.docker.com/get-started/
```
- Полезные команды:
`$ sudo docker images` # Вывести список всех images
`$ sudo docker ps` # Список работающих контейнеров
`$ sudo docker ps -a` # Список всех контейнеров
`$ sudo docker run -it ubuntu bash` # Запустить CLI Ubuntu
