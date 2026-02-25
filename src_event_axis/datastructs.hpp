#pragma once

#include "rowMatClass.h"
#include "defaults.hpp"

#include <sys/time.h>
#include <vector>
#include <string>

#include <cstdint>
typedef uint8_t uchar;


// 1. CAMERA PARAMS: Упрощаем настройки, фиксируем Y800
struct CamParams {
	int FrameW, FrameH, MAX_STAT_SIZE_BYTES;
	std::string ID_CAM;

	CamParams() {
		FrameW = FRAME_WIDTH;
		FrameH = FRAME_HEIGHT;
		MAX_STAT_SIZE_BYTES = MAX_STATISTICS_FILE_SIZE_MB * BYTES_IN_MB 
		ID_CAM = "Traffic Detector";
	}

	// static std::string params2text(const CamParams &pars);
	// static CamParams readParamsFromFile(bool &isOpened, std::string fname);
};


// 2. DATA STORAGE: Оптимизировано под Y800 (одна плоскость)
class Frame : public RowMatX<uchar> {
public:
	// FramePt теперь не содержит вектор! Только прямой указатель.
    // Это радикально ускоряет доступ к пикселям в циклах.
	struct FramePt {
		const uchar* p; 

		inline void operator++() { ++p; }
		inline uchar operator*() const { return *p; }
	}; 

	struct timeval t;

	Frame();
	Frame clone() const;

	// Упрощенный метод получения указателя на строку
	inline const uchar* getRowPtr(size_t row, size_t col = 0) const { 
		return mats[0][row] + col; 
	}
};


// 3. GEOMETRY: Без изменений, так как это математика
struct TraffPoint {
	int x, y;
	TraffPoint() : x(0), y(0) { };
	TraffPoint(int xNew, int yNew) : x(xNew), y(yNew) {};
};

struct TraffRect {
	int x1, y1, x2, y2, width, height;
	TraffRect() : x1(0), y1(0), x2(0), y2(0), width(0), height(0) {};
	TraffRect(int x1New, int y1New, int x2New, int y2New)
		: x1(x1New), y1(y1New), x2(x2New), y2(y2New), 
          width(x2New - x1New + 1), height(y2New - y1New + 1) {};
};

class TraffPolygon {
private:
	TraffRect boundingRect;
	std::vector<TraffPoint> pointList;

public:
	void setPointList(const std::vector<TraffPoint> &list);
	TraffRect getBoundingRect() const;
	bool containsPoint(const TraffPoint &pt) const;

private:
	void isect_line(int &winding, const TraffPoint &p1, const TraffPoint &p2, const TraffPoint &pos) const;
	void computeBoundingRect();
};
