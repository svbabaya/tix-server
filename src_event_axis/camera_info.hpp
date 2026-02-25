#pragma once

#include <string>

class CameraInfo {
public:
    std::string model;
    std::string ip;
    std::string serial;
    std::string appName;
    std::string appVersion;
    std::string firmware;
    std::string ntpServer;
    std::string currentTime;
    std::string cpuLoad;
    std::string memFree;
    
    // pthread_mutex_t lock;

    std::string toJson() const {
        return "{"
               "\"model\": \"" + model + "\", "
               "\"ip\": \"" + ip + "\", "
               "\"serial\": \"" + serial + "\", "
               "\"app\": \"" + appName + "\", "
               "\"ver\": \"" + appVersion + "\", "
               "\"firmware\": \"" + firmware + "\", "
               "\"ntp\": \"" + ntpServer + "\", "
               "\"camera_time\": \"" + currentTime + "\", "
               "\"cpu_load\": \"" + cpuLoad + "\", "
               "\"mem_free\": \"" + memFree + "\""
               "}";
    }
};

class InfoCollector {
public:
    static CameraInfo collect();
private:
    static std::string exec(const char* cmd);
};
