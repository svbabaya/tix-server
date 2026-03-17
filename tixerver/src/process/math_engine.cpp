#include "math_engine.hpp"
#include "adapter.hpp"
#include "capture_factory.hpp" 
#include "algo_params.hpp"

#include <syslog.h>
#include <unistd.h>
#include <memory>   // C++11: для smart pointers

/**
 * Точка входа в поток. 
 * В C++11 std::thread позволяет передавать AppContext* напрямую.
 */
void MathEngine::run_thread(AppContext* ctx) {
    if (!ctx) {
        syslog(LOG_ERR, "MathEngine: Context is null, cannot start.");
        return;
    }
    processing_loop(ctx);
}

/**
 * Основной цикл обработки
 */
void MathEngine::processing_loop(AppContext* ctx) {
    Adapter adapter;

    // CaptureFactory возвращает std::unique_ptr или аналогичный объект
    auto capturer = CaptureFactory::create();

    // Проверка инициализации захвата видео
    if (!capturer || !capturer->open(ctx->capParams)) {
        syslog(LOG_ERR, "MathEngine: Capture initialization failed (No implementation or open error)!");
        return;
    }
    
    syslog(LOG_NOTICE, "MathEngine: Processing started (Multi-Sensor Mode)");

    // Используем атомарный метод load() для безопасной проверки флага из другого потока
    while (ctx->running.load()) {
        Frame frame = capturer->handle();
        
        if (!frame.empty()) {
            // ШАГ 1: Забираем снимок настроек (Snapshot) для текущего кадра.
            // Метод getSnapshot() сам внутри захватывает и отпускает мьютекс
            GlobalConfig currentCfg = ctx->algoSettings.getSnapshot();
            
            // ШАГ 2: Обновляем настройки алгоритмов
            adapter.updateSettings(currentCfg);

            // ШАГ 3: Обработка кадра
            adapter.processFrame(frame);
            
            // ШАГ 4: Синхронизация результатов. 
            // ВАЖНО: Внутри syncResultsIfNeeded должен быть std::lock_guard<std::mutex>
            adapter.syncResultsIfNeeded(ctx->results);

        } else {
            // Безопасная пауза потока на 20 миллисекунд для разгрузки CPU при отсутствии кадров
            usleep(20000); 
        }
    }

    // Сюда попадем после того, как основной поток вызовет ctx->stop()
    syslog(LOG_NOTICE, "MathEngine: Thread exiting gracefully.");

    // Явное освобождение ресурсов захвата
    capturer->close(); 
}
