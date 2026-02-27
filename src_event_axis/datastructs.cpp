#ifndef __ROWMAT_CLASS_H__
#define __ROWMAT_CLASS_H__

#include <vector>
#include <cstring>
#include <cmath>
#include <cstdint>
#include <algorithm>

// Безусловное включение OpenCV
#include <opencv2/core/core.hpp>

typedef uint8_t uchar;
typedef double _FloatType; 

// Координаты (Y, X) - полная интеграция с cv::Point
struct PointYX {
    int y, x;
    PointYX() : y(0), x(0) {}
    PointYX(int _y, int _x) : y(_y), x(_x) {}
    PointYX(const cv::Point &cvPt) : y(cvPt.y), x(cvPt.x) {}
    operator cv::Point() const { return cv::Point(x, y); }
};

// Класс матрицы для Y800 (8-bit Gray)
class RowMat {
protected:
    size_t *refcounter;
    bool external_data;
    uchar *data;
    uchar **rows;
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
            other.hh = other.ww = 0;
        }
        return *this;
    }

    bool create(size_t h, size_t w) {
        cleanup();
        if (h == 0 || w == 0) return false;
        hh = h; ww = w;
        refcounter = new size_t(1);
        data = new uchar[hh * ww](); 
        rows = new uchar*[hh];
        for (size_t i = 0; i < hh; ++i) rows[i] = data + i * ww;
        return true;
    }

    // Zero-copy из cv::Mat
    bool fromCvMat(const cv::Mat& img, bool copydata = false) {
        cleanup();
        if (img.empty() || img.type() != CV_8UC1) return false;
        hh = (size_t)img.rows; ww = (size_t)img.cols;
        refcounter = new size_t(1);
        if (copydata) {
            external_data = false;
            data = new uchar[hh * ww];
            for (size_t y = 0; y < hh; ++y) 
                std::memcpy(data + y * ww, img.ptr<uchar>(y), ww);
        } else {
            external_data = true;
            data = const_cast<uchar*>(img.data);
        }
        rows = new uchar*[hh];
        for (size_t i = 0; i < hh; ++i) rows[i] = data + i * ww;
        return true;
    }

    // Представление для OpenCV (без копирования)
    cv::Mat toCvView() const { 
        return empty() ? cv::Mat() : cv::Mat((int)hh, (int)ww, CV_8UC1, data); 
    }

    RowMat clone() const {
        RowMat out(hh, ww);
        if (data && out.data) std::memcpy(out.data, data, hh * ww);
        return out;
    }

    inline bool empty() const { return refcounter == nullptr; }
    inline size_t height() const { return hh; }
    inline size_t width() const { return ww; }
    inline uchar* operator[](size_t y) { return rows[y]; }
    inline const uchar* operator[](size_t y) const { return rows[y]; }
    inline uchar* ptr() { return data; }

    // Расчет яркости
    void getMeanStdDev(_FloatType &mean, _FloatType &stdDev) const {
        if (empty()) return;
        double sum = 0, sq_sum = 0;
        size_t total = hh * ww;
        for (size_t i = 0; i < total; ++i) {
            double v = (double)data[i];
            sum += v; sq_sum += v * v;
        }
        mean = sum / total;
        double var = (sq_sum / total) - (mean * mean);
        stdDev = (var > 0) ? std::sqrt(var) : 0;
    }
};

#endif
