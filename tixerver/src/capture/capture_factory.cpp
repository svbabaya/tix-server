#include "capture_factory.hpp"

#if defined(CAMERA_AXIS)
    #include "capture_axis.hpp"
#elif defined(CAMERA_STUB)
    #include "capture_stub.hpp"
#elif defined(CAMERA_HIK)
    // #include "capture_hik.hpp"
#endif

std::unique_ptr<CaptureBase> CaptureFactory::create() {
#if defined(CAMERA_AXIS)
    return std::unique_ptr<CaptureBase>(new CaptureAxis());

#elif defined(CAMERA_STUB)
    return std::unique_ptr<CaptureBase>(new CaptureStub());

// #elif defined(CAMERA_HIK)
    // return std::unique_ptr<CaptureBase>(new CaptureHik());

#else
    // Если при компиляции не задан ни один тип камеры (-D...)
    // Возвращаем nullptr, чтобы MathEngine мог корректно обработать ошибку
    return nullptr;
#endif
}
