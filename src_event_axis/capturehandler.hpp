#pragma once

#include "rowMatClass.hpp"
#include "defaults.hpp"
#include <capture.h>

class CaptureBase {
public:
    virtual ~CaptureBase() = default;
    virtual void close() = 0;
    virtual bool open(int w = FRAME_WIDTH, int h = FRAME_HEIGHT) = 0;
    virtual Frame handle() = 0;
};

class CaptureY800 : public CaptureBase {
private:
    media_stream *source = nullptr;
    Frame data; 
    int w = 0, h = 0;

public:
    CaptureY800() = default;
    ~CaptureY800() override; 

    void close() override;
    bool open(int frameW = FRAME_WIDTH, int frameH = FRAME_HEIGHT) override;
    Frame handle() override;
};
