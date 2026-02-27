#include "rowMatClass.hpp"
#include <cstring>
#include <algorithm>

// --- PointYX и TraffRect ---
PointYX::PointYX() : y(0), x(0) {}
PointYX::PointYX(int _y, int _x) : y(_y), x(_x) {}
PointYX::PointYX(const cv::Point &cvPt) : y(cvPt.y), x(cvPt.x) {}
PointYX::operator cv::Point() const { return cv::Point(x, y); }

TraffRect::TraffRect() : x1(0), y1(0), x2(0), y2(0), width(0), height(0) {}
TraffRect::TraffRect(int x1N, int y1N, int x2N, int y2N) 
    : x1(x1N), y1(y1N), x2(x2N), y2(y2N), width(x2N - x1N + 1), height(y2N - y1N + 1) {}

// --- RowMat Ядро ---
RowMat::RowMat() : refcounter(nullptr), external_data(false), data(nullptr), rows(nullptr), hh(0), ww(0) {}
RowMat::RowMat(size_t h, size_t w) : RowMat() { create(h, w); }
RowMat::~RowMat() { cleanup(); }

void RowMat::cleanup() {
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

RowMat::RowMat(const RowMat& other) : refcounter(other.refcounter), external_data(other.external_data), 
    data(other.data), rows(other.rows), hh(other.hh), ww(other.ww) {
    if (refcounter) ++(*refcounter);
}

RowMat::RowMat(RowMat&& other) noexcept : refcounter(other.refcounter), external_data(other.external_data), 
    data(other.data), rows(other.rows), hh(other.hh), ww(other.ww) {
    other.refcounter = nullptr; other.data = nullptr; other.rows = nullptr; other.hh = other.ww = 0;
}

RowMat& RowMat::operator=(const RowMat& other) {
    if (this != &other) { cleanup(); refcounter = other.refcounter; external_data = other.external_data; 
        data = other.data; rows = other.rows; hh = other.hh; ww = other.ww; if (refcounter) ++(*refcounter); }
    return *this;
}

RowMat& RowMat::operator=(RowMat&& other) noexcept {
    if (this != &other) { cleanup(); refcounter = other.refcounter; external_data = other.external_data; 
        data = other.data; rows = other.rows; hh = other.hh; ww = other.ww; 
        other.refcounter = nullptr; other.data = nullptr; other.rows = nullptr; other.hh = other.ww = 0; }
    return *this;
}

bool RowMat::create(size_t h, size_t w) {
    cleanup();
    if (h == 0 || w == 0) return false;
    hh = h; ww = w;
    refcounter = new size_t(1);
    data = new uchar[hh * ww](); 
    rows = new uchar*[hh];
    for (size_t i = 0; i < hh; ++i) rows[i] = data + i * ww;
    return true;
}

cv::Mat RowMat::toCvView() const { 
    return empty() ? cv::Mat() : cv::Mat((int)hh, (int)ww, CV_8UC1, data); 
}

RowMat RowMat::clone() const {
    RowMat out(hh, ww);
    if (data && out.data) std::memcpy(out.data, data, hh * ww);
    return out;
}

void RowMat::getMeanStdDev(_FloatType &mean, _FloatType &stdDev) const {
    if (empty() || hh == 0 || ww == 0) return;
    double sum = 0, sq_sum = 0;
    size_t total = hh * ww;
    for (size_t i = 0; i < total; ++i) {
        double v = (double)data[i]; sum += v; sq_sum += v * v;
    }
    mean = sum / total;
    double var = (sq_sum / total) - (mean * mean);
    stdDev = (var > 0) ? std::sqrt(var) : 0;
}

// --- Реализация Frame ---
Frame::Frame() { t.tv_sec = 0; t.tv_usec = 0; }
Frame::Frame(Frame&& other) noexcept : RowMat(std::move(other)), t(other.t) {}
Frame& Frame::operator=(Frame&& other) noexcept {
    if (this != &other) { RowMat::operator=(std::move(other)); t = other.t; }
    return *this;
}
Frame Frame::clone() const {
    Frame out; static_cast<RowMat&>(out) = RowMat::clone(); out.t = this->t;
    return out;
}

// --- Реализация TraffPolygon ---
void TraffPolygon::setPointList(const std::vector<PointYX> &list) { pointList = list; computeBoundingRect(); }

bool TraffPolygon::containsPoint(const PointYX &pt) const {
    if (pointList.empty()) return false;
    if (pt.x < boundingRect.x1 || pt.x > boundingRect.x2 || pt.y < boundingRect.y1 || pt.y > boundingRect.y2) return false;
    int winding = 0;
    for (size_t i = 0; i < pointList.size(); ++i) 
        isect_line(winding, pointList[i], pointList[(i + 1) % pointList.size()], pt);
    return (winding % 2 != 0);
}

void TraffPolygon::isect_line(int &winding, const PointYX &p1, const PointYX &p2, const PointYX &pos) const {
    if ((p1.y <= pos.y && p2.y > pos.y) || (p2.y <= pos.y && p1.y > pos.y)) {
        float vt = (float)(pos.y - p1.y) / (p2.y - p1.y);
        if (pos.x < p1.x + vt * (p2.x - p1.x)) winding++;
    }
}

void TraffPolygon::computeBoundingRect() {
    if (pointList.empty()) { boundingRect = TraffRect(); return; }
    int minx = pointList[0].x, maxx = minx, miny = pointList[0].y, maxy = miny;
    for (const auto& p : pointList) {
        if (p.x < minx) minx = p.x; if (p.x > maxx) maxx = p.x;
        if (p.y < miny) miny = p.y; if (p.y > maxy) maxy = p.y;
    }
    boundingRect = TraffRect(minx, miny, maxx, maxy);
}
