#pragma once

#include <string>

struct CaptureConfig {
    int width = 640;
    int height = 480;
    int fps = 10;
    std::string format = "Y800"; // Других форматов не предусмотрено
};
