#include "math_results.hpp"

#include <algorithm> // Для std::fill

MathResults::MathResults() {
    // В C++11 значения уже заданы в hpp, но если логика требует 
    // полного сброса при создании — вызываем resetStat
    resetStat();
}

void MathResults::resetStat() {
    // Автоматическая блокировка на время работы метода
    std::lock_guard<std::mutex> guard(lock);

    time.clear();
    counter = speed = 0;
    k = 0.0f;
    occup = 0;
    
    // Сброс индивидуальных переменных
    Cnt0 = Cnt1 = Cnt2 = Cnt3 = Cnt4 = 0;
    Spd0 = Spd1 = Spd2 = Spd3 = Spd4 = 0;
    
    gap = 0.0f;
    headway = 0;
    negcnt = 0;
    totalCounter = 0;
    carDetected = carDetected_sensor = false;
    curCounter = curSpeed = 0;
    
    // Безопасное обнуление массива
    std::fill(std::begin(curCnt), std::end(curCnt), 0);
    
    curLength = -1.0f;
    curClass = "-";
    lastAvgCounter = 0;
    
    // Временные поля для тестирования
    objects_detected = 0;
    last_score = 0.0;
}
