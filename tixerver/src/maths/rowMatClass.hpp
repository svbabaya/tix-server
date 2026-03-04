#ifndef ROWMATCLASS_HPP
#define ROWMATCLASS_HPP

#include <vector>
#include <cstdint>
#include <cmath>
#include <sys/time.h>
#include <opencv2/core/core.hpp>

typedef uint8_t uchar;
typedef double _FloatType;

struct PointYX {
    int y, x;
    PointYX();
    PointYX(int _y, int _x);
    PointYX(const cv::Point &cvPt);
    operator cv::Point() const;
};

struct TraffRect {
    int x1, y1, x2, y2, width, height;
    TraffRect();
    TraffRect(int x1N, int y1N, int x2N, int y2N);
};

class TraffPolygon {
private:
    TraffRect boundingRect;
    std::vector<PointYX> pointList;
public:
    void setPointList(const std::vector<PointYX> &list);
    TraffRect getBoundingRect() const { return boundingRect; }
    bool containsPoint(const PointYX &pt) const;
private:
    void isect_line(int &winding, const PointYX &p1, const PointYX &p2, const PointYX &pos) const;
    void computeBoundingRect();
};

class RowMat {
protected:
    size_t *refcounter;
    bool external_data;
    uchar *data;
    uchar **rows;
    size_t hh, ww;
    void cleanup();

public:
    RowMat();
    RowMat(size_t h, size_t w);
    virtual ~RowMat();

    RowMat(const RowMat& other);
    RowMat(RowMat&& other) noexcept;
    RowMat& operator=(const RowMat& other);
    RowMat& operator=(RowMat&& other) noexcept;

    bool create(size_t h, size_t w);
    cv::Mat toCvView() const;
    RowMat clone() const;
    void release();
    void getMeanStdDev(_FloatType &mean, _FloatType &stdDev) const;

    inline bool empty() const { return refcounter == nullptr; }
    inline size_t height() const { return hh; }
    inline size_t width() const { return ww; }
    inline uchar* operator[](size_t y) { return rows[y]; }
    inline const uchar* operator[](size_t y) const { return rows[y]; }
    inline uchar* ptr() { return data; }
    inline const uchar* ptr() const { return data; }
};

class Frame : public RowMat {
public:
    struct FramePt {
        const uchar* p;
        inline void operator++() { ++p; }
        inline const uchar* operator[](size_t) const { return p; }
    };

    struct timeval t;
    bool rgb, yuv; // не используются только для совместимости со старой версией

    Frame();
    Frame(const Frame& other) = default;
    Frame& operator=(const Frame& other) = default;
    Frame(Frame&& other) noexcept;
    Frame& operator=(Frame&& other) noexcept;

    Frame clone() const;
    void getRowPts(FramePt &st, size_t row, size_t col = 0) const;
    
    // Оптимизированный доступ для одномерного цикла
    inline const uchar* begin() const { return ptr(); }
    inline const uchar* end() const { return ptr() + (hh * ww); }
};

#endif
