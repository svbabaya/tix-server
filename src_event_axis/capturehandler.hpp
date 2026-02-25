#pragma once

#include <capture.h>
#include "datastructs.h"

class CaptureBase {
protected:
    Frame data;
public:
    CaptureBase() {}
    virtual ~CaptureBase() {}
    virtual void close() = 0;
    virtual bool open(int frameW, int frameH) = 0;
    virtual const Frame handle() = 0;
};

class CaptureY800 : public CaptureBase {
private:
    media_stream *source;
    int w, h;

public:
    CaptureY800();
    ~CaptureY800();

    void close() override;
    bool open(int frameW, int frameH) override;
    const Frame handle() override;

private:
    CaptureY800(const CaptureY800&);
    CaptureY800& operator=(const CaptureY800&);
};
