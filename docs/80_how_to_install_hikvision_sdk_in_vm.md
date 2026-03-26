## How to install and use Hikvision SDK in Virtual Mashine
Установка Hikvision SDK в WSL (Windows) или LiMa (Mac OS). Подходит для установки в различных сборках Linux.
В документе "HEOP2.0_Developer Guide.pdf" установка Docker рассчитана на старые версии Ubuntu и выглядит немного иначе.

SDK Hikvision загружается как docker image, поэтому один из возможных вариантов дальнейших действий в MacOs, где ранее уже была установлен LiMa, использовать Colima (Containers on Lima) - инструмент с открытым исходным кодом, который позволяет запускать контейнеры Docker и Kubernetes на macOS (и Linux), альтернатива Docker Desktop. Colima использует Lima как основу, добавляя к нему удобный интерфейс для работы с контейнерами.

Обратите внимание, что при использовании Colima значительная часть действий из руководства не потребуется, поскольку Colima берет их на себя по умолчанию (создание виртуальной машины в LiMa с предустановленным Docker, настроенными ключами и репозиторием). Действия при установке SDK на чистый Ubuntu сервер приведены в конце этого документа.

- В системе должен быть установлен LiMa (06_how_to)
- Устанавливаем Colima (из командной строки MacOs):
```
$ brew install colima
```
- Запускаем Colima:
```
$ colima start
```
- Проверяем, что Docker работает:
```
$ docker ps
```
Должен появится пустой список контейнеров `CONTAINER ID   IMAGE     COMMAND   CREATED   STATUS    PORTS     NAMES`
- Подключаемся к виртуальной машите Colima:
```
$ colima ssh
```
Теперь вы внутри Linux:
```
sergebabayan@colima:/Users/sergebabayan$ 
```
Далее все команды выполняются внутри виртуальной машины.

- Перед тем, как открыть файл конфигурации Docker, возможно придется установить текстовый редактор, например, nano:
```
$ sudo apt update
$ sudo apt install nano -y
```
- Открываем файл конфигурации:
```
$ sudo nano /etc/docker/daemon.json и вставляем в него этот код:
```
Добавляем настройку. Если в фигурных скобках уже есть другие настройки, после них нужно поставить запятую:
```
{
   ...,
   "insecure-registries": ["13.251.8.106"]
}
```
- Сохраняем изменения.
- Перезапускаем Docker внутри виртуальной машины:
```
$ sudo systemctl restart docker
```
- Выходим из виртуальной машины:
```
$ exit
```

### 2. Загружаем Docker image SDK Hikvision
Для этого на портале Hikvision нужно получить логин, пароль и ссылку для загрузки image под определенную модель камеры:
2.1 Авторизуемся на портале **tpp.hikvision.com**
2.2 Заходим в **HEOP MANAGER - HEOP Overview - Download Docker**
2.3 Находим в списке нужную камеру

В окне появится информация для авторизации и скачивания docker image, пример:
```
DS-2CD3146G2-ISU(2.8 mm)(H) # Выбранная из списка камера
Username: docker-global-prod
Password: ...
Docker Image URL: 13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240
```
- В основном терминале MacOs с помощью username и password заходим в Docker Hub:
```
$ sudo docker login 13.251.8.106
Username: docker-global-prod
Password: ...
```
При успешной авторизации появится сообщение `Login Succeeded`
- Скачиваем image:
```
$ docker pull 13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240
```


/??????
- После скачивания появится информация:
```
Digest: sha256:c1f43eb9f5a6625437e1774f420ef7ae6ec37491f93ef132d11636d99a54848b
Status: Downloaded newer image for 13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240
13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240
```
Image можно увидеть по команде:
```
$ docker images
IMAGE                                                    ID             DISK USAGE   CONTENT SIZE
13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240  f0785bc2711b   13.8GB       0B
```
Но при этом нужно знать в каком контексте вы находитесь, поскольку если у вас установлен Docker Desktop, Docker внутри LiMa, Docker как Colima, эта команда покажет разную информацию в зависимости от того, какой Docker активен в данный момент. Docker CLI — это просто "пульт", который подключается к активному демону (Docker Engine). Где находится этот демон — туда и смотрит команда.
- Проверяем какой демон активен (отмечен *):
```
$ docker context ls
NAME              DESCRIPTION                               DOCKER ENDPOINT
colima            colima                                    unix:///Users/sergebabayan/.colima/default/docker.sock
default           Current DOCKER_HOST based configuration   unix:///var/run/docker.sock
desktop-linux *   Docker Desktop                            unix:///Users/sergebabayan/.docker/run/docker.sock
```
Кстати, удалить контекст можно так: `$ docker context rm {NAME}`

- Переключаемся на контекст Colima: `docker context use colima` или `colima start` если Colima ранее была остановлена.

### Как перенести образ из Docker MacOs в Colima
- Переключаем контекст на Docker Desktop
```
$ docker context use desktop-linux
```
- Архивируем нужный образ:
```
$ docker save -o ~/Desktop/hikvision-sdk.tar 13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240
```
- Запускаем Colima:
```
$ colima start
```
- Загружаем образ в Colima:
```
$ docker load -i ~/Desktop/hikvision-sdk.tar
```
В тоге должно появиться сообщение:
```
Loaded image: 13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240
```
### Далее можно работать с SDK

Запускаем контейнер из этого образа:
```
$ docker run -it --user root --entrypoint /bin/sh 13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240
```
В результате должно появиться приглашение Linux:
```
sh-5.0# 
```
Далее знакомимся с SDK















### Информация Docker - LiMa - Colima
Docker Desktop
├── Docker CLI (на macOS)
└── Linux VM (скрытая, создаётся Docker Desktop)
    └── Docker Engine

Colima
├── Docker CLI (на macOS, тот же самый)
└── Lima VM (создаётся Colima)
    └── Docker Engine (устанавливается Colima внутри VM)


┌────────────────────────────────────────────────────────┐
│                      macOS                             │
│  ┌─────────────────────────────────────────────────┐   │
│  │              Docker CLI (клиент)                │   │
│  │         (команда docker из Homebrew)            │   │
│  └─────────────────────────────────────────────────┘   │
│                         │                              │
│                         │ подключение                  │
│                         ▼                              │
│  ┌─────────────────────────────────────────────────┐   │
│  │              Lima (виртуализация)               │   │
│  │  ┌─────────────────────────────────────────┐    │   │
│  │  │      Виртуальная машина Linux           │    │   │
│  │  │  ┌───────────────────────────────────┐  │    │   │
│  │  │  │   Docker Engine (демон)           │  │    │   │
│  │  │  │   dockerd                         │  │    │   │
│  │  │  └───────────────────────────────────┘  │    │   │
│  │  └─────────────────────────────────────────┘    │   │
│  └─────────────────────────────────────────────────┘   │
└────────────────────────────────────────────────────────┘







### Optional
- Запустить контейнер в интерактивном режиме:
```
$ docker run -it 13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240 bash
```
- Сохранить образ в tar-файл
```
$ docker save -o hikvision-sdk.tar 13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240
```
Файл появится в текущей директории `ls -lh hikvision-sdk.tar`
- Сжать для передачи:
```
$ gzip hikvision-sdk.tar
```
- На другом компьютере загрузить образ:
```
$ docker load -i hikvision-sdk.tar.gz
```
- Смонтировать локальную папку для работы с проектами:
```
$ docker run -it -v /Users/ваше_имя/projects:/workspace 13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240 bash
```
- Создать alias для удобного запуска:
```
$ alias hik-sdk='docker run -it -v $(pwd):/workspace 13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240'
```
- Узнать архитектуру image:
```
$ docker inspect 13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240 | grep Architecture
```



### First version
### 1. Устанавливаем Docker в Linux (wsl или lima)
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

### 2. Загружаем Docker image SDK Hikvision
2.1 Авторизуемся на портале **tpp.hikvision.com**
2.2 Заходим в **HEOP MANAGER - HEOP Overview - Download Docker**
2.3 Находим в списке нужную камеру

В окне появится информация для авторизации и скачивания docker image, пример:
```
DS-2CD3146G2-ISU(2.8 mm)(H) # Выбранная из списка камера
Username: docker-global-prod
Password: ...
Docker Image URL: 13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240
```
2.4 Заходим в Hikvision Docker Hub:
```
$ sudo docker login 13.251.8.106
Username: docker-global-prod
Password: ...
```
При успешной авторизации появится сообщение `Login Succeeded`

Если авторизация не произошла и появилась ошибка типа: `Error response from daemon: Get "https://13.251.8.106/v2/": EOF` делаем следующее:

Создаем (открываем существующий) файл конфигурации Docker `$ sudo nano /etc/docker/daemon.json` и вставляем в него этот код:
```
{
  "insecure-registries": ["13.251.8.106"]
}
```
Сохраняем файл, перезапускаем Docker `$ sudo systemctl restart docker` и снова пытаемся зайти в Hikvision Docker Hub с помощью Username и Password.

2.5 Загружаем image командой:
```
$ sudo docker pull 13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240
```
После этого должна начаться загрузка, которая может занять длительное время.

Когда образ будет загружен, появится примерно такое сообщение:
```
Digest: sha256:c1f43eb9f5a6625437e1774f420ef7ae6ec37491f93ef132d11636d99a54848b
Status: Downloaded newer image for 13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240
13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240
```

Командой `$ sudo docker images` можно проверить, что image успешно установлен.

### 3. Копируем скрипт heop_devel_run.sh

- Загрузите скрипт «heop_devel_run.sh» из архива "G5 HEOP Developer Materials.zip" в корневую директорию проекта
- Добавьте разрешение на исполнение `$ sudo chmod +x heop_devel_run.sh`

Этот скрипт нужно запускать перед сборкой приложения `$ sudo ./heop_devel_run.sh -r`

### Информация о скрипте

Скрипт создает и запускает Docker контейнер с HEOP окружением для разработки приложений под камеры Hikvision (PSH firmware).

При первом запуске:
- Скрипт проверит наличие Docker
- Создаст директорию ~/heop_devel_kit для хранения данных
- Монтирует локальную директорию (VOLUME_PATH) в /heop/workspace внутри контейнера: запросит путь для volume (где будут храниться наши файлы). Нажимаем Enter для использования пути по умолчанию: ~/heop_devel_kit/volume но лучше вводим свой путь:
```
Please enter your volume path(user data path)[enter means uses default value /home/user/heop_devel_kit/volume]:
/Users/sergebabayan/vscode-workspace/tix-server/tixerver
```
- Покажет список доступных Docker образов HEOP и попросит выбрать
- Запустит контейнер со смонтированной директорией
