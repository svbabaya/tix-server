#pragma once

#include "capture_base.hpp"

class CaptureStub : public CaptureBase {
public:
    virtual ~CaptureStub() noexcept override {}
    bool open(int w, int h) override { return true; }
    void close() override {}
    Frame handle() override { return Frame(); } // Пустой кадр
};
