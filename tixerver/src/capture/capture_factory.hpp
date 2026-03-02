#ifndef CAPTURE_FACTORY_HPP
#define CAPTURE_FACTORY_HPP

#include <memory>
#include <string>
#include "capture_base.hpp"

class CaptureFactory {
public:
    /**
     * Создает экземпляр камеры на основе строки типа.
     * Возвращает умный указатель, который сам удалит объект.
     */
    static std::unique_ptr<CaptureBase> create(const std::string& type);
};

#endif // CAPTURE_FACTORY_HPP
