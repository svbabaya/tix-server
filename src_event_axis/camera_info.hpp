#ifndef CAMERA_INFO_HPP
#define CAMERA_INFO_HPP

#include <string>

class CameraInfo {
public:
    std::string model;
    std::string serial;
    std::string firmware;
    std::string ip;
    std::string appName;
    std::string appVersion;
    std::string currentTime;
    std::string cpuLoad;
    std::string memFree;
    std::string ntpServer;

    // pthread_mutex_t lock;

    std::string toJson() const {
        return "{"
               "\"model\": \"" + model + "\", "
               "\"serial\": \"" + serial + "\", "
               "\"firmware\": \"" + firmware + "\", "
               "\"ip\": \"" + ip + "\", "
               "\"ntp\": \"" + ntpServer + "\", "
               "\"app\": \"" + appName + "\", "
               "\"ver\": \"" + appVersion + "\", "
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

#endif // CAMERA_INFO_HPP
