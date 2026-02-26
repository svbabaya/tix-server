#pragma once

#include <vector>
#include <cstring>
#include <cmath>
#include <cstdint>
#include <algorithm>

#include <opencv2/core/core.hpp>

typedef uint8_t uchar;
typedef double _FloatType; 

// Координаты (Y, X) - полная интеграция с cv::Point
struct PointYX {
    int y, x;
    PointYX() : y(0), x(0) {}
    PointYX(int _y, int _x) : y(_y), x(_x) {}

    // Конструкторы для мгновенного преобразования
    PointYX(const cv::Point &cvPt) : y(cvPt.y), x(cvPt.x) {}
    operator cv::Point() const { return cv::Point(x, y); }
};

template <typename TElem>
class RowMat {
protected:
    size_t *refcounter;
    bool external_data;
    TElem *data;
    TElem **rows;
    size_t hh, ww;

    void cleanup() {
        if (refcounter) {
            if (--(*refcounter) == 0) {
                if (!external_data && data) delete[] data;
                if (rows) delete[] rows;
                delete refcounter;
            }
        }
        refcounter = nullptr; data = nullptr; rows = nullptr;
        external_data = false; hh = ww = 0;
    }

public:
    RowMat() : refcounter(nullptr), external_data(false), data(nullptr), rows(nullptr), hh(0), ww(0) {}
    RowMat(size_t h, size_t w) : RowMat() { create(h, w); }
    virtual ~RowMat() { cleanup(); }

    // Конструктор копирования (Refcounting)
    RowMat(const RowMat& other) {
        refcounter = other.refcounter; external_data = other.external_data;
        data = other.data; rows = other.rows; hh = other.hh; ww = other.ww;
        if (refcounter) ++(*refcounter);
    }

    // C++11 Move Constructor
    RowMat(RowMat&& other) noexcept 
        : refcounter(other.refcounter), external_data(other.external_data), 
          data(other.data), rows(other.rows), hh(other.hh), ww(other.ww) {
        other.refcounter = nullptr; other.data = nullptr; other.rows = nullptr;
        other.hh = other.ww = 0;
    }

    RowMat& operator=(const RowMat& other) {
        if (this != &other) { cleanup();
            refcounter = other.refcounter; external_data = other.external_data;
            data = other.data; rows = other.rows; hh = other.hh; ww = other.ww;
            if (refcounter) ++(*refcounter);
        }
        return *this;
    }

    RowMat& operator=(RowMat&& other) noexcept {
        if (this != &other) { cleanup();
            refcounter = other.refcounter; external_data = other.external_data;
            data = other.data; rows = other.rows; hh = other.hh; ww = other.ww;
            other.refcounter = nullptr; other.data = nullptr; other.rows = nullptr;
        }
        return *this;
    }

    bool create(size_t h, size_t w) {
        cleanup();
        if (h == 0 || w == 0) return false;
        hh = h; ww = w;
        refcounter = new size_t(1);
        data = new TElem[hh * ww](); 
        rows = new TElem*[hh];
        for (size_t i = 0; i < hh; ++i) rows[i] = data + i * ww;
        return true;
    }

    // Создание RowMat из cv::Mat (Zero-copy по умолчанию)
    bool fromCvMat(const cv::Mat& img, bool copydata = false) {
        cleanup();
        if (img.empty()) return false;
        hh = img.rows; ww = img.cols;
        refcounter = new size_t(1);
        if (copydata) {
            external_data = false;
            data = new TElem[hh * ww];
            for (size_t y = 0; y < hh; ++y) 
                std::memcpy(data + y * ww, img.ptr<TElem>(y), ww * sizeof(TElem));
        } else {
            external_data = true;
            data = (TElem*)img.data;
        }
        rows = new TElem*[hh];
        for (size_t i = 0; i < hh; ++i) rows[i] = data + i * ww;
        return true;
    }

    // Обратное преобразование: RowMat -> cv::Mat (без копирования)
    cv::Mat toCvView() { 
        return empty() ? cv::Mat() : cv::Mat(hh, ww, cv::DataType<TElem>::type, data); 
    }

    RowMat clone() const {
        RowMat out(hh, ww);
        if (data && out.data) std::memcpy(out.data, data, hh * ww * sizeof(TElem));
        return out;
    }

    inline bool empty() const { return refcounter == nullptr; }
    inline size_t height() const { return hh; }
    inline size_t width() const { return ww; }
    inline TElem* operator[](size_t y) { return rows[y]; }
    inline const TElem* operator[](size_t y) const { return rows[y]; }
    inline TElem* ptr() { return data; }
};

// Контейнер каналов (наследник std::vector для Y800)
template <typename TElem>
class RowMatX {
protected:
    std::vector<RowMat<TElem>> mats;
public:
    RowMatX() = default;
    virtual ~RowMatX() = default;
    RowMatX(RowMatX&&) noexcept = default;
    RowMatX& operator=(RowMatX&&) noexcept = default;

    void release() { mats.clear(); }
    void push(const RowMat<TElem>& m) { mats.push_back(m); }
    void push(RowMat<TElem>&& m) { mats.push_back(std::move(m)); }
    inline bool empty() const { return mats.empty(); }
    inline size_t size() const { return mats.size(); }
    inline RowMat<TElem>& operator[](size_t i) { return mats[i]; }
    inline const RowMat<TElem>& operator[](size_t i) const { return mats[i]; }
};
