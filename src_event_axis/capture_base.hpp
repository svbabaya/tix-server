#pragma once

#include "rowMatClass.hpp"

class CaptureBase {
public:
    virtual ~CaptureBase() noexcept = default;
    virtual void close() = 0;
    virtual bool open(const CaptureConfig& cfg) = 0;
    virtual Frame handle() = 0;
};
