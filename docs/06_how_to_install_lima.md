## How to install LiMa on MacOs

LiMa (Linux on Mac) позволяет запускать очень лёгкую Linux-виртуалку, внутри которой можно запускать Docker (через утилиту Colima). Это менеджер виртуальных машин для командной строки, созданный специально для разработчиков, которым нужно быстро получить изолированное Linux-окружение.

- Устанавливаем LiMa
```
$ brew install lima
```
По умолчанию создается виртуальная машина default из которой уже есть доступ к директориям MacOs

- Запускаем LiMa
```
$ limactl start
```
- Заходим в командную оболочку LiMa, по сути в командную строку Linux
```
$ lima
```
Вы увидите что-то похожее:
```
sergebabayan@lima-default:/Users/sergebabayan$ 
```
Здесь sergebabayan - это username в MacOs, то есть при запуске вы оказываетесь в домашней директории MacOs, но в LiMa у вас есть другая домашняя директория /home/sergebabayan.linux, в которую рекомендуется устанавливать Axis SDK

- Выйти из LiMa
```
$ exit
```
- Остановить LiMa
```
$ limactl stop
```
Теперь необходимо решить - где будет расположен проект приложения для камеры. Варианты - в MacOS или в LiMa. Если в MacOs (так удобнее работать с проектом в IDE), то нужно разрешить LiMa вносить изменения в файлы MacOS, поскольку по умолчанию LiMa может только читать такие файлы. Это нужно делать из командной строки MacOS. То есть выходим из LiMa, останавливаем LiMa, разрешаем вносить изменения:
```
$ limactl edit default --mount-writable
``` 
Теперь снова запускаем LiMa и заходим в командную оболочку:
```
limactl start
lima
```
Если проект будет размещен внутри LiMa, то для клонирования его с GitHub потребуется создать подключение LiMa к github через SSH, кроме того, чтобы открыть проект в IDE VSC, установленный в MacOs, придется установить в VSC сервер SSH с плагином C/C++ для работы с удаленным проектом. Этот вариант в руководстве не рассматривается.

Чтобы была возможность загружать пакет на Axis из Lima с помощью скрипта eap-install.sh, необходимо настроить Lima для работы в режиме host network:
- Выходим из Lima в macOS:
```
exit
```
- Устанавливаем socket_vmnet:
```
brew install socket_vmnet
```
- Уточняем где находится socket_vmnet:
```
ls $(brew --prefix)/opt/socket_vmnet/bin/socket_vmnet
```
- Получаем результат (это должен быть симлинк):
```
/usr/local/opt/socket_vmnet/bin/socket_vmnet
```
- Находим реальный адрес по симлинку:
```
readlink -f /usr/local/opt/socket_vmnet/bin/socket_vmnet
```
- Получаем результат:
```
/usr/local/Cellar/socket_vmnet/1.2.2/bin/socket_vmnet
```
- Создаем отдельную директорию для socket_vmnet:
```
sudo mkdir -p /opt/socket_vmnet/bin
```
- Копируем туда socket_vmnet (это поможет избежать проблем в Lima):
```
sudo cp /usr/local/Cellar/socket_vmnet/1.2.2/bin/socket_vmnet /opt/socket_vmnet/bin/
```
- Передаем права root новой папке:
```
sudo chown -R root:wheel /opt/socket_vmnet
```
- Открываем (создаем) файл конфигурации:
```
nano ~/.lima/_config/networks.yaml
```
- Вставляем (меняем) строки:
```
paths:
  socketVMNet: "/opt/socket_vmnet/bin/socket_vmnet"
...
bridged:
    mode: bridged
    interface: en8
```
В строке interface указываем тот интерфейс у которому подключена камера, в данном случае это Ethernet адаптер.
- Открываем конфигурацию:
```
limactl edit default
```
- Добавляем (редактируем) эти настройки в секции `networks:`, обращаем внимание на пробелы, чтобы не нарушать формат yaml:
```
networks:
  - lima: bridged
    interface: en8  # Проверяем имя адаптера через ifconfig
```
При успешном сохранении файла конфигурации появится предложение сразу запустить Lima, но лучше сделать это позднее отдельной командой `limactl start default`

- Запускаем создание правил sudoers:
```
limactl sudoers | sudo tee /etc/sudoers.d/lima
```
Если limactl sudoers сработала правильно и вывела содержимое конфигурации для sudo, значит Lima приняла новый путь и права доступа подтверждены. При выводе должна быть похожая строка, означающая что включен режим bridged и интерфейс en8:
```
/opt/socket_vmnet/bin/socket_vmnet --pidfile=/private/var/run/lima/bridged_socket_vmnet.pid --socket-group=everyone --vmnet-mode=bridged --vmnet-interface=en8 /private/var/run/lima/socket_vmnet.bridged...
```
- Запускаем Lima:
```
limactl start
```
- Проверяем процессы на Mac:
```
ps aux | grep "bridged --vmnet-interface=en8"
```
- Должны появиться похожие строки:
```
sergebabayan     59437   0.0  0.0 34121056    712 s000  S+    4:51AM   0:00.01 grep bridged --vmnet-interface=en8
root             58725   0.0  0.0 34174576   1092 s000  T     4:48AM   0:00.01 /opt/socket_vmnet/bin/socket_vmnet --pidfile=/private/var/run/lima/bridged_socket_vmnet.pid --socket-group=everyone --vmnet-mode=bridged --vmnet-interface=en8 /private/var/run/lima/socket_vmnet.bridged
root             58724   0.0  0.0 34151468   2312 s000  T     4:48AM   0:00.01 sudo --user root --group wheel --non-interactive /opt/socket_vmnet/bin/socket_vmnet --pidfile=/private/var/run/lima/bridged_socket_vmnet.pid --socket-group=everyone --vmnet-mode=bridged --vmnet-interface=en8 /private/var/run/lima/socket_vmnet.bridged
```
- Заходим в оболочку Lima:
```
limactl shell default
```
Прописываем IP для сети с камерой:
```
sudo ip addr add 169.254.211.10/16 dev lima0
```
Проверяем связь:
```
ping -c 4 169.254.211.120
```

















