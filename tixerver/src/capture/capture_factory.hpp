#ifndef CAPTURE_FACTORY_HPP
#define CAPTURE_FACTORY_HPP

#include "capture_base.hpp"

#include <memory>

class CaptureFactory {
public:
    /**
     * Создает экземпляр камеры.
     * Возвращает умный указатель, который сам удалит объект.
     */
    static std::unique_ptr<CaptureBase> create();
};

#endif // CAPTURE_FACTORY_HPP
