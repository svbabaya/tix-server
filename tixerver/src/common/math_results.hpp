#ifndef MATH_RESULTS_HPP
#define MATH_RESULTS_HPP

#include <string>
#include <mutex>

struct MathResults {
    // mutable позволяет блокировать мьютекс в константных методах (если появятся)
    mutable std::mutex lock;

    // Statistics members (Legacy TraffStat)
    std::string time;
    int counter = 0;
    int speed = 0;
    float k = 0.0f;
    int occup = 0;
    
    // Сохраняем индивидуальные имена для совместимости с другими модулями
    int Cnt0 = 0, Cnt1 = 0, Cnt2 = 0, Cnt3 = 0, Cnt4 = 0;
    int Spd0 = 0, Spd1 = 0, Spd2 = 0, Spd3 = 0, Spd4 = 0;
    
    float gap = 0.0f;
    int headway = 0;
    int negcnt = 0;
    int totalCounter = 0;
    bool carDetected = false, carDetected_sensor = false;
    int curCounter = 0, curSpeed = 0;
    int curCnt[5] = {0, 0, 0, 0, 0};
    float curLength = -1.0f;
    std::string curClass = "-";
    int lastAvgCounter = 0;

    // Конструктор C++11
    MathResults();
    
    // Деструктор больше не нужен для очистки мьютекса, std::mutex сделает это сам
    ~MathResults() = default;

    void resetStat();

    // Временные поля для тестирования
    int objects_detected = 0;
    double last_score = 0.0;
};

#endif // MATH_RESULTS_HPP
