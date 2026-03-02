#ifndef CAPTURE_STUB_HPP
#define CAPTURE_STUB_HPP

#include "capture_base.hpp"

// #include "sdk_libs_other_camera"

class CaptureStub : public CaptureBase {
private:
    // media_stream_data_type_new_camera *source = nullptr;
    Frame data; 
    int w = 0, h = 0;

public:
    virtual ~CaptureStub() noexcept override {
        close();
    }

    bool open(const CaptureParams& cfg) override;
    void close() override;
    Frame handle() override;
};

#endif // CAPTURE_STUB_HPP
