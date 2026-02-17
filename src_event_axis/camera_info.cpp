#include "camera_info.hpp"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <vector>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>

// Вспомогательная функция для выполнения системных команд
std::string InfoCollector::exec(const char* cmd) {
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "unknown";
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        result += buffer;
    }
    pclose(pipe);
    // Удаляем лишний перевод строки
    if (!result.empty() && result[result.length()-1] == '\n') 
        result.erase(result.length()-1);
    return result;
}

CameraInfo InfoCollector::collect() {
    CameraInfo info;

    // 1. Модель и серийный номер (через системные файлы Axis)
    // Обычно лежат в /etc/axis-release или через parhand
    info.model = exec("grep PROD_SHORTNAME /etc/axis-release | cut -d'=' -f2");
    info.serial = exec("cat /sys/class/net/eth0/address | tr -d ':'");

    // 2. IP адрес (через ioctl для eth0)
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct ifreq ifr;
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);
    info.ip = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);

    // 3. Название и версия программы (из вашего package.conf при сборке)
    // В реальности эти данные можно захардкодить в макросы через Makefile
    // info.appName = "TiXerver";
    // info.appVersion = "1.0.1";

    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    info.currentTime = buf;

    // Сбор данных о памяти
    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    if (meminfo.is_open()) {
        while (std::getline(meminfo, line)) {
            if (line.find("MemFree:") == 0) {
                // Извлекаем числовое значение (например, "MemFree: 12345 kB")
                info.memFree = line.substr(8); 
                // Убираем лишние пробелы в начале
                info.memFree.erase(0, info.memFree.find_first_not_of(" \t"));
                break;
            }
        }
        meminfo.close();
    } else {
        info.memFree = "unknown";
    }

    // Данные из макросов Makefile (package.conf)
    info.appName = APP_NAME_STR;
    info.appVersion = APP_VER_STR;

    return info;
}
