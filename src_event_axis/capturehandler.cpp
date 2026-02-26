#include "capturehandler.hpp"
#include <cstdio>
#include <cstring>
#include <algorithm>

// Деструктор наследника вызывается первым. 
// Он должен корректно закрыть дескрипторы SDK до разрушения базового класса.
CaptureY800::~CaptureY800() {
    close();
}

void CaptureY800::close() {
    // 1. Освобождаем ресурсы захвата камеры
    if (source != nullptr) {
        capture_close_stream(source);
        source = nullptr;
    }
    
    // 2. Очищаем буфер кадра в оперативной памяти
    // data.release() вызовет cleanup() в RowMat, уменьшив счетчик ссылок.
    data.release();
    
    // 3. Обнуляем параметры состояния
    w = 0;
    h = 0;
    data.t.tv_sec = 0;
    data.t.tv_usec = 0;
}

bool CaptureY800::open(int frameW, int frameH) {
    // Гарантируем закрытие предыдущей сессии перед открытием новой
    close();

    if (frameW <= 0 || frameH <= 0) {
        return false;
    }

    w = frameW;
    h = frameH;

    char cap_prop[256];
    // Попытка №1: Запрашиваем чистый Y800 (Gray 8-bit)
    std::snprintf(cap_prop, sizeof(cap_prop), "resolution=%dx%d&sdk_format=Y800", w, h);
    source = capture_open_stream(IMAGE_UNCOMPRESSED, cap_prop);

    if (source == nullptr) {
        // Попытка №2: Если камера не дает Y800, берем NV12. 
        // В NV12 первый слой (Y) — это те же 8 бит яркости, что нам нужны.
        std::snprintf(cap_prop, sizeof(cap_prop), "resolution=%dx%d&sdk_format=NV12", w, h);
        source = capture_open_stream(IMAGE_UNCOMPRESSED, cap_prop);
    }

    if (source == nullptr) {
        return false;
    }

    // Выделяем память под буфер один раз при открытии.
    // data наследует RowMat::create(), который аллоцирует сплошной блок памяти.
    if (!data.create(h, w)) {
        close();
        return false;
    }

    return true;
}

Frame CaptureY800::handle() {
    // Если поток не открыт, возвращаем пустой объект
    if (source == nullptr) {
        return Frame();
    }

    // Запрашиваем кадр из SDK
    media_frame *frame = capture_get_frame(source);
    if (!frame) {
        return Frame();
    }

    // Получаем сырой указатель на данные кадра
    const uchar* fData = reinterpret_cast<const uchar*>(capture_frame_data(frame));
    
    if (fData != nullptr) {
        // 1. Копируем временную метку (Timestamp) из SDK в структуру Frame
        capture_time ct = capture_frame_timestamp(frame);
        const long long NS_IN_SEC = 1000000000LL;
        data.t.tv_sec = ct / NS_IN_SEC;
        data.t.tv_usec = (ct % NS_IN_SEC) / 1000;

        // 2. Копируем пиксели яркости (Y-канал)
        // stride — это ширина строки в байтах в памяти SDK (может включать отступы для выравнивания)
        int stride = capture_frame_stride(frame);

        if (stride == 0 || stride == w) {
            // Оптимальный случай: данные лежат плотно, копируем всё одним блоком
            std::memcpy(data.ptr(), fData, (size_t)h * w);
        } else {
            // Данные в SDK имеют "поля" (padding), копируем построчно
            for (int y = 0; y < h; ++y) {
                std::memcpy(data[y], fData + (y * stride), (size_t)w);
            }
        }
    }

    // Освобождаем дескриптор кадра в SDK (но данные уже скопированы в наш data)
    capture_frame_free(frame);

    // Возвращаем копию через Refcounting (быстро) или Move (еще быстрее в C++11)
    // Благодаря наследованию от RowMat, здесь не происходит глубокого копирования пикселей.
    return data;
}
