#include "capture_factory.hpp"

// Macros CAMERA_AXIS, CAMERA_DAHUA, CAMERA_HIK definit in makefile.{camera_type}

#if defined(CAMERA_AXIS)
    #include "capture_axis.hpp"
#elif defined(CAMERA_STUB)
    #include "capture_stub.hpp"
#elif defined(CAMERA_DAHUA)
    // #include "capture_dahua.hpp"    
#elif defined(CAMERA_HIK)
    // #include "capture_hik.hpp"
#endif

std::unique_ptr<CaptureBase> CaptureFactory::create() {
#if defined(CAMERA_AXIS)
    return std::unique_ptr<CaptureBase>(new CaptureAxis());

#elif defined(CAMERA_STUB)
    return std::unique_ptr<CaptureBase>(new CaptureStub());

// #elif defined(CAMERA_DAHUA)
    // return std::unique_ptr<CaptureBase>(new CaptureDahua());

// #elif defined(CAMERA_HIK)
    // return std::unique_ptr<CaptureBase>(new CaptureHik());

#else
    // Если при компиляции не задан ни один тип камеры (-D...)
    // Возвращаем nullptr, чтобы MathEngine мог корректно обработать ошибку
    return nullptr;
#endif
}
