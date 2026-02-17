#pragma once

#include <string>

class CameraInfo {
public:
    std::string model;
    std::string serial;
    std::string ip;
    std::string appName;
    std::string appVersion;
    std::string currentTime;
    std::string memFree;

    std::string toJson() const {
        return "{"
               "\"model\": \"" + model + "\", "
               "\"serial\": \"" + serial + "\", "
               "\"ip\": \"" + ip + "\", "
               "\"app\": \"" + appName + "\", "
               "\"version\": \"" + appVersion + "\", "
               "\"time\": \"" + currentTime + "\""
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
