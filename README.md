# camera-server
```mermaid
graph TD
    %% Определение узлов
    Start((Начало main)) --> InitLibevent[Инициализация libevent base]
    InitLibevent --> CreateContext[Создание AppContext app]
    
    CreateContext --> NetStart{Запуск NetworkServer?}
    
    NetStart -- Ошибка --> ExitErr[syslog: Critical Error & Exit]
    NetStart -- Успех --> MathThread[pthread_create: MathEngine]
    
    %% Разделение на потоки
    subgraph MainThread [Основной поток: Сеть]
        MathThread --> Detach[pthread_detach]
        Detach --> Dispatch[event_base_dispatch]
        Dispatch --> Loop[Цикл обработки HTTP и TCP событий]
    end

    subgraph MathProcess [Поток MathEngine]
        MathThread -.-> Run[run_thread]
        Run --> ProcLoop[processing_loop]
        ProcLoop --> Analyze[analyze: имитация нагрузки]
        Analyze --> Update[Запись результатов в AppContext]
        Update --> Sleep[usleep 100ms]
        Sleep --> ProcLoop
    end

    %% Взаимодействие через контекст
    Loop -.->|Чтение/Запись| CreateContext
    ProcLoop -.->|Чтение/Запись| CreateContext

    %% Оформление
    style CreateContext fill:#f9f,stroke:#333,stroke-width:2px
    style MainThread fill:#e1f5fe,stroke:#01579b
    style MathProcess fill:#fff3e0,stroke:#e65100
```
Как читать эту схему:
AppContext (розовый блок): Это центральное хранилище (Shared Context), через которое общаются два независимых потока.
Синий блок (Main Thread): Здесь крутится libevent. Он отвечает на ваши запросы GET_MATH и GET /api/info. Он "спит", пока не придет пакет по сети.
Оранжевый блок (Math Process): Это тот самый цикл, который вы настраивали. Он работает параллельно и постоянно грузит процессор вычислениями.
Пунктирные стрелки: Показывают доступ к общим данным под защитой мьютексов.

Диаграмма последовательности (Sequence Diagram)
Показать именно логику запроса (например, как данные попадают из MathEngine к вам в Telnet):

```mermaid
sequenceDiagram
    participant M as MathEngine Thread
    participant C as AppContext (Shared)
    participant N as Network (libevent)
    participant U as User (Telnet/HTTP)

    Note over M: Постоянный цикл 10 FPS
    M->>C: Запись results (objects_detected)
    
    U->>N: Команда "GET_MATH"
    N->>C: Чтение results (lock mutex)
    C-->>N: Данные (count, score)
    N->>U: JSON Response
```
