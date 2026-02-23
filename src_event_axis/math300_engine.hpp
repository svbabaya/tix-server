#ifndef MATH300_ENGINE_HPP
#define MATH300_ENGINE_HPP

#include "app300_context.hpp"

#include <capture.h>

class MathEngine {
public:
    // Точка входа для pthread_create (вызывается из main)
    static void* run_thread(void* arg);

private:
    // Бесконечный цикл: захват кадров -> вызов analyze -> запись результатов
    static void processing_loop(AppContext* ctx);

    // Чистая математическая логика (принимает настройки, возвращает результат)
    // Мы передаем настройки по ссылке, чтобы не копировать лишнего
    static int analyze(media_frame* frame, const VideoSettings& current_cfg); 
};

#endif
