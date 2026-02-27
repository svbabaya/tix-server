#include "capturehandler.hpp"
#include <cstdio>
#include <cstring>

CaptureY800::~CaptureY800() {
    close();
}

void CaptureY800::close() {
    if (source) {
        capture_close_stream(source);
        source = nullptr;
    }
    data.release();
    w = 0;
    h = 0;
}

bool CaptureY800::open(int frameW, int frameH) {
    close();
    w = frameW; h = frameH;

    char cap_prop[128];
    // Используем CAPTURE_FORMAT и CAPTURE_FPS из defaults.hpp
    std::snprintf(cap_prop, sizeof(cap_prop), 
                 "resolution=%dx%d&sdk_format=%s&fps=%d", 
                 w, h, CAPTURE_FORMAT.c_str(), CAPTURE_FPS);

    source = capture_open_stream(IMAGE_UNCOMPRESSED, cap_prop);
    
    if (!source) {
        // Лог ошибки, если SDK не приняло параметры
        return false;
    }

    return data.create(h, w);
}

Frame CaptureY800::handle() {
    if (!source) return Frame();
    
    media_frame *frame = capture_get_frame(source);
    if (!frame) return Frame();

    const uchar* fData = (const uchar*)capture_frame_data(frame);
    if (fData) {
        capture_time ct = capture_frame_timestamp(frame);
        data.t.tv_sec = ct / 1000000000;
        data.t.tv_usec = (ct % 1000000000) / 1000;

        int stride = capture_frame_stride(frame);
        if (stride == 0 || stride == w) {
            std::memcpy(data.ptr(), fData, (size_t)h * w);
        } else {
            for (int y = 0; y < h; ++y)
                std::memcpy(data[y], fData + (y * stride), (size_t)w);
        }
    }
    
    capture_frame_free(frame);
    // Возвращаем объект (Refcounting / Move)
    return data; 
}
