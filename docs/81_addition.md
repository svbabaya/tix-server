## Addition for 80_how_to_install_hikvision_sdk_in_vm.md

SDK загружается c портала Hikvision как docker image, поэтому в первую очередь необходимо установить Docker Engine. Самый простой способ - установка Docker Desktop, который доступен для macOS, Windows и некоторых дистрибутивов Linux. При установке Docker Desktop вы получаете Docker Engine + GUI. В этом проекте используется другой подход, подразумевающий установку и использование Docker Engine через Docker CLI. Docker CLI не является Docker Desktop. Это отдельная программа нашей операционной системы.
Docker CLI мы вызываем в терминале командами docker ps, docker run и т.д., то есть она работает как пульт управления для Docker Engine.

### macOS
Непосредственно в MacOs, без установки Docker Desktop, нельзя установить Docker Engine. Поэтому используем LiMa (Linux on Mac). LiMa создает и управляет виртуальными машинами Linux на macOS. По сути, это "движок", который поднимает среду Linux. А в дополнение к LiMa устанавливаем Colima, которая создает Docker Engine. Docker CLI на macOS подключается к этому Engine.

- Устанавливаем LiMa + Colima + Docker CLI:
```
brew install colima docker // эта команда автоматически установит lima
```
Проверяем установку:
```
sergebabayan@Serges-MacBook-Pro ~ % limactl --version
limactl version 2.0.3
sergebabayan@Serges-MacBook-Pro ~ % colima --version
colima version 0.10.1
sergebabayan@Serges-MacBook-Pro ~ % docker --version
Docker version 29.3.1, build c2be9ccfc3
```
- Запускаем среду:
```
colima start
```
По умолчанию Colima выделит 2 ядра CPU, 2 GiB RAM и 100 GiB диска, но можно указать другие значения:
```
colima start --cpu 4 --memory 8 --disk 50
```
- Проверяем установку Docker Engine:
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

- Проверяем контекст, то есть какой именно Docker Engine активен в данный момент:
```
docker context ls
```
Результат:
```
NAME            DESCRIPTION                               DOCKER ENDPOINT                                          ERROR
colima *        colima                                    unix:///Users/sergebabayan/.colima/default/docker.sock   
default         Current DOCKER_HOST based configuration   unix:///var/run/docker.sock                              
desktop-linux   Docker Desktop                            unix:///Users/sergebabayan/.docker/run/docker.sock   
```
После команды `colima start` контекст Colima автоматически становится активным. Может пригодиться: удаление контекста: `docker context rm {NAME}`, переключение контекста: `docker context use colima`













### Windows


### Linux
Установка терминальной версии Desktop Engine