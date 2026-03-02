#ifndef CAPTURE_BASE_HPP
#define CAPTURE_BASE_HPP

#include "rowMatClass.hpp"
#include "capture_params.hpp"

class CaptureBase {
public:
    virtual ~CaptureBase() noexcept {}
    virtual void close() = 0;
    virtual bool open(const CaptureParams& cfg) = 0;
    virtual Frame handle() = 0;
};

#endif // CAPTURE_BASE_HPP
