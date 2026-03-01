#include "capture_axis.hpp"

#include <cstdio>
#include <cstring>


void CaptureAxis::close() {
    if (source) {
        capture_close_stream(source);
        source = nullptr;
    }
    data.release();
    w = 0;
    h = 0;
}

bool CaptureAxis::open(const CaptureConfig& cfg) {
    close();
    w = cfg.width; 
    h = cfg.height;

    char cap_prop[128];
    // Все данные берем из переданной структуры cfg
    std::snprintf(cap_prop, sizeof(cap_prop), 
                 "resolution=%dx%d&sdk_format=%s&fps=%d", 
                 w, h, cfg.format.c_str(), cfg.fps);

    source = capture_open_stream(IMAGE_UNCOMPRESSED, cap_prop);
    
    if (!source) return false;
    return data.create(h, w);
}


Frame CaptureAxis::handle() {
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
    return data.clone(); 
}
