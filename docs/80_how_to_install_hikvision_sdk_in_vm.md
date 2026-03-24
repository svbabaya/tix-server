## How to install and use Hikvision SDK in Virtual Mashine
Установка Hikvision SDK в WSL (Windows) или LiMa (Mac OS). Подходит для установки в различных сборках Linux.
В документе "HEOP2.0_Developer Guide.pdf" установка Docker расчитана на старые версии Ubuntu и выглядит немного иначе.

### 1. Устанавливаем Docker в Linux (wsl или lima)
1.1 Добляем ключ Docker
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

Когда образ будет загружен, появится примерно такое сообщение:
```
Digest: sha256:c1f43eb9f5a6625437e1774f420ef7ae6ec37491f93ef132d11636d99a54848b
Status: Downloaded newer image for 13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240
13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240
```

Командой `$ sudo docker images` можно проверить, что image успешно установлен.
