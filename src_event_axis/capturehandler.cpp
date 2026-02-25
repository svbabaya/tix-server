#include "capturehandler.h"
#include <cstring>

CaptureBase::~CaptureBase() {
    close();
}

void CaptureBase::close() {
    data.release();
    data.t.tv_sec = data.t.tv_usec = 0;
    data.rgb = data.yuv = false;
}

CaptureY800::CaptureY800() : source(NULL), w(0), h(0) {}

CaptureY800::~CaptureY800() {
    close();
}

void CaptureY800::close() {
    CaptureBase::close();
    if (source != NULL) {
        capture_close_stream(source);
        source = NULL;
    }
}

bool CaptureY800::open(int frameW, int frameH) {
    close();
    if (frameW <= 0 || frameH <= 0) return false;
    
    w = frameW;
    h = frameH;

    char cap_prop[128];
    // Запрашиваем Y800 напрямую у SDK камеры
    snprintf(cap_prop, 128, "resolution=%dx%d&sdk_format=Y800", w, h);

    source = capture_open_stream(IMAGE_UNCOMPRESSED, cap_prop);
    if (source == NULL) {
        // Если камера не поддерживает Y800, пробуем NV12 (яркость там лежит так же)
        snprintf(cap_prop, 128, "resolution=%dx%d&sdk_format=NV12", w, h);
        source = capture_open_stream(IMAGE_UNCOMPRESSED, cap_prop);
    }

    if (source == NULL) return false;

    // Выделяем память только под одну плоскость (Яркость)
    data.push(RowMat<uchar>(h, w));
    data.rgb = data.yuv = false;

    return true;
}

const Frame CaptureY800::handle() {
    if (source == NULL) return Frame();

    media_frame *frame = capture_get_frame(source);
    if (!frame) return Frame();

    const unsigned char* fData = (const unsigned char*)capture_frame_data(frame);
    if (!fData) {
        capture_frame_free(frame);
        return Frame();
    }

    // 1. Оптимизированное получение времени (без лишних делений в цикле)
    capture_time ct = capture_frame_timestamp(frame);
    data.t.tv_sec = ct / 1000000000;
    data.t.tv_usec = (ct % 1000000000) / 1000;

    // 2. Копирование данных
    const size_t rowsize = w * sizeof(unsigned char);
    const int stride = capture_frame_stride(frame);

    // Если данные лежат плотно (stride == ширина), копируем всё за один раз (максимальная скорость)
    if (stride == 0 || stride == (int)rowsize) {
        memcpy(data[0][0], fData, h * rowsize);
    } else {
        // Если есть отступы (stride), копируем построчно
        const unsigned char *pIn = fData;
        for (int yy = 0; yy < h; ++yy) {
            memcpy(data[0][yy], pIn, rowsize);
            pIn += stride;
        }
    }

    capture_frame_free(frame);
    return data;
}
