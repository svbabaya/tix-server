#ifndef CAPTURE_CONFIG_HPP
#define CAPTURE_CONFIG_HPP

#include <string>

struct CaptureConfig {
    int width = 640;
    int height = 480;
    int fps = 10;
    std::string format = "Y800"; // Других форматов не предусмотрено
};

#endif // CAPTURE_CONFIG_HPP
