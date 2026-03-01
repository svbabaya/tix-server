#pragma once

#include "rowMatClass.hpp"

#include "capture_config.hpp"

class CaptureBase {
public:
    virtual ~CaptureBase() noexcept {}
    virtual void close() = 0;
    virtual bool open(const CaptureConfig& cfg) = 0;
    virtual Frame handle() = 0;
};
