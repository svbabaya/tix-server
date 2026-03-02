#ifndef CAPTURE_PARAMS_HPP
#define CAPTURE_PARAMS_HPP

#include <string>

struct CaptureParams {
    int width = 640;
    int height = 480;
    int fps = 10;
    std::string format = "Y800"; // Других форматов пока не предусмотрено
};

#endif // CAPTURE_PARAMS_HPP
