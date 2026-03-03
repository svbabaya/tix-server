## main.cpp

Диаграмма последовательности (Sequence Diagram) показывает разделение на Основной поток (Network/Libevent) и Математический поток (pthread).
```mermaid
sequenceDiagram
    participant OS as Operating System
    participant Main as Main Thread (Libevent)
    participant Ctx as AppContext (g_ctx)
    participant Net as NetworkServer
    participant Math as MathEngine Thread

    Main->>OS: openlog()
    Main->>Main: event_base_new()
    Main->>Ctx: Initialize AppContext(base)
    Main->>OS: signal(SIGINT/SIGTERM, handler)
    
    Main->>Net: start(api_port, data_port)
    Note over Net: Listening for TCP/HTTP
    
    Main->>OS: pthread_create(MathEngine::run_thread)
    activate Math
    Note over Math: Running analysis loop
    
    Main->>Main: event_base_dispatch(base)
    Note right of Main: BLOCKING: Handling Network Events
    
    alt Signal Received (SIGINT/SIGTERM)
        OS->>Main: signal_handler()
        Main->>Ctx: g_ctx->stop() (sets flag)
        Main->>Main: event_base_loopbreak()
    end

    Main->>Main: Exit dispatch loop
    Main->>Math: pthread_join()
    deactivate Math
    Note over Math: Thread finished
    
    Main->>Main: event_base_free()
    Main->>OS: closelog()
    Main->>OS: return 0
```

Диаграмма архитектуры и потоков данных (Flowchart)
```mermaid
graph TD
    Start((Start)) --> InitLog[openlog / syslog]
    InitLog --> Libevent[Init libevent: event_base_new]
    Libevent --> Context[Create AppContext & g_ctx]
    Context --> Sig[Setup Signal Handlers]
    
    subgraph Parallel_Execution [Параллельная работа]
        direction TB
        Net[NetworkServer: Listening Ports]
        Dispatch[Main Thread: event_base_dispatch]
        MathThread[MathEngine: pthread_run_thread]
    end

    Sig --> Net
    Net --> MathThread
    MathThread --> Dispatch

    Dispatch -- "SIGINT / SIGTERM" --> Handler[Signal Handler]
    Handler --> StopFlag[Set AppContext::stop = true]
    StopFlag --> Break[event_base_loopbreak]
    
    Break --> Join[pthread_join: Wait for Math]
    Join --> Cleanup[Free Resources & closelog]
    Cleanup --> End((Exit 0))

    style Dispatch fill:#f9f,stroke:#333,stroke-width:2px
    style MathThread fill:#bbf,stroke:#333,stroke-width:2px
    style Handler fill:#ff9,stroke:#333,stroke-width:4px
```

Блокировка: event_base_dispatch помечен как блокирующий вызов основного потока.
Асинхронность: Видно, что MathEngine работает независимо в pthread_t.
Безопасный выход: Показана цепочка Signal -> Stop Flag -> Loopbreak -> Join, которая гарантирует, что ресурсы не будут освобождены, пока математический поток не закончит работу.

Диаграмма взаимодействия потоков через AppContext
Для визуализации взаимодействия между NetworkServer и MathEngine через AppContext (мьютексы и общие данные), на диаграмму добавлены механизмы синхронизации.
В данной архитектуре AppContext выступает в роли Shared Memory, а pthread_mutex_t гарантирует, что сетевой поток не прочитает данные в момент их записи математическим движком.

```mermaid
sequenceDiagram
    participant Net as Network Thread (libevent)
    box "Shared Resources (AppContext)"
        participant Mutex as pthread_mutex (lock)
        participant Data as GlobalResults / Config
    end
    participant Math as MathEngine Thread (pthread)

    Note over Net, Math: Приложение запущено. Потоки работают параллельно.

    rect rgb(240, 248, 255)
    Note right of Math: Цикл обработки кадра
    Math->>Math: processFrame()
    Math->>Mutex: pthread_mutex_lock()
    activate Mutex
    Math->>Data: Обновление MathResults (objects_detected, score)
    Note over Data: Данные актуальны
    Math->>Mutex: pthread_mutex_unlock()
    deactivate Mutex
    end

    rect rgb(255, 240, 245)
    Note left of Net: Внешний HTTP запрос (GET /status)
    Net->>Mutex: pthread_mutex_lock()
    activate Mutex
    Note right of Net: Ожидание, если Math еще пишет
    Net->>Data: Чтение MathResults для JSON ответа
    Net->>Mutex: pthread_mutex_unlock()
    deactivate Mutex
    Net-->>Net: Отправка ответа клиенту
    end

    rect rgb(240, 255, 240)
    Note left of Net: Внешний запрос на изменение настроек (POST /config)
    Net->>Mutex: pthread_mutex_lock()
    activate Mutex
    Net->>Data: Обновление GlobalConfig (ROI, Пороги)
    Net->>Mutex: pthread_mutex_unlock()
    deactivate Mutex
    
    Note right of Math: Следующая итерация цикла
    Math->>Data: Проверка флага обновления настроек
    Math->>Math: updateSettings(newConfig)
    end
```

Ключевые архитектурные решения на схеме:
Защита критической секции: Оба потока обращаются к pthread_mutex_lock перед доступом к MathResults (результаты) или GlobalConfig (настройки). Это предотвращает Race Condition.
Событийная модель (Net): Сетевой поток "спит" в libevent до прихода запроса, в то время как MathEngine работает в непрерывном цикле.
Атомарность обновлений: Благодаря блокировке, сетевой клиент никогда не получит "наполовину записанный" результат (например, когда objects_detected уже обновился, а last_score еще нет).
