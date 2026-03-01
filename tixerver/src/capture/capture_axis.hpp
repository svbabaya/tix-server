#ifndef CAPTURE_AXIS_HPP
#define CAPTURE_AXIS_HPP

#include "capture_base.hpp"

#include <capture.h>

class CaptureAxis : public CaptureBase {
private:
    media_stream *source = nullptr;
    Frame data; 
    int w = 0, h = 0;

public:
    virtual ~CaptureAxis() noexcept {
        close();
    }

    bool open(const CaptureConfig& cfg) override;
    void close() override;
    Frame handle() override;
};

#endif // CAPTURE_AXIS_HPP
