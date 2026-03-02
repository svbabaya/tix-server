#include "capture_factory.hpp"
#include "capture_axis.hpp"
#include "capture_stub.hpp"

std::unique_ptr<CaptureBase> CaptureFactory::create(const std::string& type) {
    // В C++11 используем конструктор std::unique_ptr(new T)

    if (type == "axis") {
        return std::unique_ptr<CaptureBase>(new CaptureAxis());
    } 
    
    if (type == "stub") {
        return std::unique_ptr<CaptureBase>(new CaptureStub());
    }

    // Add new type: if (type == "hikvision")...

    // По умолчанию или при ошибке возвращаем заглушку, 
    // чтобы MathEngine не упал от нулевого указателя
    return std::unique_ptr<CaptureBase>(new CaptureStub());
}
