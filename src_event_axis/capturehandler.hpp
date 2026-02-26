#pragma once
#include "datastructs.hpp"
#include <capture.h>

class CaptureBase {
protected:
    Frame data;
public:
    virtual ~CaptureBase() = default;
    virtual void close() = 0;
    virtual bool open(int w, int h) = 0;
    virtual Frame handle() = 0;
};

class CaptureY800 : public CaptureBase {
private:
    media_stream *source = nullptr;
    int w = 0, h = 0;
public:
    CaptureY800() = default;
    ~CaptureY800() override { close(); }
    void close() override;
    bool open(int frameW, int frameH) override;
    Frame handle() override;
};
