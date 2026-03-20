## How to download and use Hikvision SDK
В ОС должен быть установле docker, проверяем: `$ docker --version`
Если установлен, появится что-то вроде `Docker version 29.2.1, build a5c7197`

1. Авторизуемся на портале **tpp.hikvision.com**
2. Заходим в **HEOP MANAGER - HEOP Overview - Download Docker**
3. Откроется окно в котором, используя поле ввода и раскрывающийся список, находим нужную камеру
4. В окне появится информация для авторизации и скачивания docker image.
5. В терминале выполняем команду:
```
$ docker login 13.251.8.106 -u docker-global-prod
```
5. Будем запрошен пароль, берем его из окна: AKCp8...
6. Если все в порядке, увидим сообщение `Login Succeeded`
7. Пользуясь данными окна, вводим в терминале команду для скачивания image:
```
$ docker pull 13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240
```
8. Когда образ будет скачан, появится примерно такое сообщение:
```
Digest: sha256:c1f43eb9f5a6625437e1774f420ef7ae6ec37491f93ef132d11636d99a54848b
Status: Downloaded newer image for 13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240
13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240
```
Далее будет предложено сделать анализ безопасности docker image, этот шаг вероятно лучше пропустить.
```
What's next:
    View a summary of image vulnerabilities and recommendations → docker scout quickview 13.251.8.106/docker-global-prod/g5/v2.3.0:G5_2507080240
```
