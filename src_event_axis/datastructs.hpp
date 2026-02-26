#pragma once
#include "rowMatClass.hpp"
#include <sys/time.h>

class Frame : public RowMat<uchar> {
public:
    struct timeval t;

    Frame() {
        t.tv_sec = 0;
        t.tv_usec = 0;
    }

    // C++11 Move Semantics
    Frame(Frame&& other) noexcept 
        : RowMat<uchar>(std::move(other)), t(other.t) {}

    Frame& operator=(Frame&& other) noexcept {
        if (this != &other) {
            RowMat<uchar>::operator=(std::move(other));
            t = other.t;
        }
        return *this;
    }

    // Глубокая копия при необходимости
    Frame clone() const {
        Frame out;
        if (!this->empty()) {
            // Копируем данные через базовый RowMat
            static_cast<RowMat<uchar>&>(out) = RowMat<uchar>::clone();
            out.t = this->t;
        }
        return out;
    }

    // Самый быстрый доступ к строке для Y800
    inline const uchar* getRowPtr(size_t row) const { 
        return (*this)[row]; 
    }
};
