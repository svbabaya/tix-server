#pragma once

#include "rowMatClass.hpp"
#include <sys/time.h>

// Теперь Frame — это ОДНА матрица яркости. Максимальная скорость.
class Frame : public RowMat<uchar> {
public:
    struct timeval t;

    Frame() { t.tv_sec = 0; t.tv_usec = 0; }
    
    // C++11 Move (очень быстро)
    Frame(Frame&& other) noexcept 
        : RowMat<uchar>(std::move(other)), t(other.t) {}
        
    Frame& operator=(Frame&& other) noexcept {
        if (this != &other) {
            RowMat<uchar>::operator=(std::move(other));
            t = other.t;
        }
        return *this;
    }

    // Если нужно создать независимую копию
    Frame clone() const {
        Frame out;
        static_cast<RowMat<uchar>&>(out) = RowMat<uchar>::clone();
        out.t = this->t;
        return out;
    }
};
