#include "camera_info.hpp"

#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <vector>
#include <cstring>
#include <string>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/wait.h>


// Вспомогательная функция для выполнения системных команд
std::string InfoCollector::exec(const char* cmd) {
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    
    if (!pipe) {
        syslog(LOG_ERR, "TiXerver: Failed to run command: %s", cmd);
        return "error_exec";
    }

    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        result += buffer;
    }

    int status = pclose(pipe);
    if (status != 0) {
        syslog(LOG_WARNING, "TiXerver: Command '%s' returned code %d", cmd, WEXITSTATUS(status));
    }

    // Удаляем \n и \r в конце строки
    while (!result.empty() && (result[result.length() - 1] == '\n' || result[result.length() - 1] == '\r')) {
        result.erase(result.length() - 1);
    }

    return result.empty() ? "unknown" : result;
}

CameraInfo InfoCollector::collect() {
    CameraInfo info;

    info.model = exec("grep 'ProdShortName' /etc/sysconfig/brand.conf | cut -d'\"' -f2");
    info.serial = exec("cat /sys/class/net/eth0/address | tr -d ':'");
    info.firmware = exec("grep 'UDHCP_OPTARGS_VENDOR_CLASS=' /etc/conf.d/udhcpc.conf | cut -d',' -f4 | tr -d '\"'");
    info.ntpServer = exec("grep 'NTPSERVER' /etc/sysconfig/openntpd.conf | cut -d\"'\" -f2");

    // IP адрес (через ioctl для eth0)
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct ifreq ifr;
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);
    info.ip = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);

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

    // Axis 1353 has 1 core. If cpuLoad=1 it is 100%
    std::ifstream loadinfo("/proc/loadavg");
    if (loadinfo.is_open()) {
        std::string avg1;
        loadinfo >> avg1; // Считываем первое число (load average за 1 минуту)

        info.cpuLoad = avg1;

        loadinfo.close();
    } else {
        info.cpuLoad = "unknown";
    }

    // Данные из макросов Makefile (package.conf)
    info.appName = APP_NAME_STR;
    info.appVersion = APP_VER_STR;

    return info;
}
