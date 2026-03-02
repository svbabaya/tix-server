#include "capture_stub.hpp"

#include <syslog.h>
#include <sys/time.h>

/**
 * Реализация заглушки (Stub).
 * Используется для тестирования логики без реального SDK.
 */

bool CaptureStub::open(const CaptureParams& cfg) {
    syslog(LOG_NOTICE, "CaptureStub: Opened in emulated mode (%dx%d, fps: %d)", 
           cfg.width, cfg.height, cfg.fps);
    return true; 
}

void CaptureStub::close() {
    syslog(LOG_NOTICE, "CaptureStub: Closed");
}

Frame CaptureStub::handle() {
    // В идеале здесь можно генерировать тестовую картинку (например, шум или полосы),
    // но для минимальной реализации просто возвращаем пустой кадр.
    // Это заставит MathEngine делать usleep(20000), как прописано в твоем цикле.
    
    Frame emptyFrame;
    
    // Если нужно имитировать метку времени для логов:
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    emptyFrame.t = tv;

    return emptyFrame; 
}
