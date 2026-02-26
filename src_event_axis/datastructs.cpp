#include "datastructs.hpp"
#include "utils_cam.h" // Предполагаем наличие i2str
#include <fstream>
#include <cstring>
#include <algorithm>

// --- CAMERA PARAMS ---
CamParams::CamParams() {
    FrameW = FRAME_WIDTH;
    FrameH = FRAME_HEIGHT;
    MAX_STATISTICS_FILE_SIZE_MB = MAX_STATISTICS_FILE_SIZE_MB_DEFAULT; 
    RGB_FRAME = 0;
    ID_CAM = "Traffic Detector";
    CAPT_TYPE = "SDK";
}

std::string CamParams::params2text(const CamParams &pars) {
    std::string config, tmp;
    config += "FrameW=" + i2str(pars.FrameW, tmp) + "\n";
    config += "FrameH=" + i2str(pars.FrameH, tmp) + "\n";
    config += "RGB_FRAME=" + i2str(pars.RGB_FRAME, tmp) + "\n";
    config += "MAX_STATISTICS_FILE_SIZE_MB=" + i2str(pars.MAX_STATISTICS_FILE_SIZE_MB, tmp) + "\n";
    config += "ID_CAM=" + pars.ID_CAM + "\n";
    config += "CAPT_TYPE=" + pars.CAPT_TYPE + "\n";
    return config;
}

CamParams CamParams::readParamsFromFile(bool &isOpened, std::string fname) {
    CamParams out;
    isOpened = false;
    std::ifstream file(fname);
    if (file.is_open()) {
        isOpened = true;
        std::string row;
        while (std::getline(file, row)) {
            size_t eq = row.find('=');
            if (eq == std::string::npos) continue;
            std::string key = row.substr(0, eq);
            std::string val = row.substr(eq + 1);

            if (key == "FrameW") out.FrameW = std::stoi(val);
            else if (key == "FrameH") out.FrameH = std::stoi(val);
            else if (key == "RGB_FRAME") out.RGB_FRAME = std::stoi(val);
            else if (key == "MAX_STATISTICS_FILE_SIZE_MB") out.MAX_STATISTICS_FILE_SIZE_MB = std::stoi(val);
            else if (key == "ID_CAM") out.ID_CAM = val;
            else if (key == "CAPT_TYPE") out.CAPT_TYPE = val;
        }
    }
    return out;
}

// --- DATA STORAGE (Frame) ---
Frame::Frame() : rgb(false), yuv(false) {
    t.tv_sec = 0;
    t.tv_usec = 0;
}

// Move constructor
Frame::Frame(Frame&& other) noexcept 
    : RowMatX<uchar>(std::move(other)), t(other.t), rgb(other.rgb), yuv(other.yuv) {}

// Move assignment
Frame& Frame::operator=(Frame&& other) noexcept {
    if (this != &other) {
        RowMatX<uchar>::operator=(std::move(other));
        t = other.t;
        rgb = other.rgb;
        yuv = other.yuv;
    }
    return *this;
}

Frame Frame::clone() const {
    Frame out;
    out.reshape(this->height(), this->width()); 
    if (!this->empty()) {
        std::memcpy(out.data(), this->data(), this->height() * this->width());
    }
    out.rgb = this->rgb;
    out.yuv = this->yuv;
    out.t = this->t;
    return out;
}

// --- GEOMETRY ---
TraffRect::TraffRect(int x1N, int y1N, int x2N, int y2N) noexcept
    : x1(x1N), y1(y1N), x2(x2N), y2(y2N) {
    width = x2 - x1 + 1;
    height = y2 - y1 + 1;
}

void TraffPolygon::setPointList(const std::vector<TraffPoint> &list) {
    this->pointList = list;
    computeBoundingRect();
}

TraffRect TraffPolygon::getBoundingRect() const {
    return boundingRect;
}

bool TraffPolygon::containsPoint(const TraffPoint &pt) const {
    if (pointList.empty()) return false;
    // Быстрая проверка по Bounding Box
    if (pt.x < boundingRect.x1 || pt.x > boundingRect.x2 || 
        pt.y < boundingRect.y1 || pt.y > boundingRect.y2) return false;

    int winding_number = 0;
    for (size_t i = 0; i < pointList.size(); ++i) {
        const auto& p1 = pointList[i];
        const auto& p2 = pointList[(i + 1) % pointList.size()];
        isect_line(winding_number, p1, p2, pt);
    }
    return (winding_number % 2 != 0);
}

void TraffPolygon::isect_line(int &winding, const TraffPoint &p1, const TraffPoint &p2, const TraffPoint &pos) const {
    if ((p1.y <= pos.y && p2.y > pos.y) || (p2.y <= pos.y && p1.y > pos.y)) {
        float vt = (float)(pos.y - p1.y) / (p2.y - p1.y);
        if (pos.x < p1.x + vt * (p2.x - p1.x)) {
            winding++;
        }
    }
}

void TraffPolygon::computeBoundingRect() {
    if (pointList.empty()) {
        boundingRect = TraffRect();
        return;
    }
    int minx = pointList[0].x, maxx = minx;
    int miny = pointList[0].y, maxy = miny;
    for (const auto& p : pointList) {
        if (p.x < minx) minx = p.x; if (p.x > maxx) maxx = p.x;
        if (p.y < miny) miny = p.y; if (p.y > maxy) maxy = p.y;
    }
    boundingRect = TraffRect(minx, miny, maxx, maxy);
}
