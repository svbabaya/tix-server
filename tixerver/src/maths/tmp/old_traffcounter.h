#ifndef OLD_TRAFFCOUNTER_H
#define OLD_TRAFFCOUNTER_H

#include "old_common.h"
#include "old_traffsensor.h"
#include "old_utils_traff.h"

#include <vector>
#include <sys/time.h>

namespace old {

class TraffCounter {
public:
	TraffCounter();
	~TraffCounter();
    
	std::vector<TraffAlgParams> getAlgParameters() const {
		std::vector<TraffAlgParams> oRet;
		for (int i = 0; i < sensorList.size(); i++)
			oRet.push_back(sensorList[i].getParams());
		return oRet;
	}
	void setAlgParameters(const std::vector<TraffAlgParams>& aNewParams) {
		assert(aNewParams.size() == sensorList.size());
		for (int i = 0; i < sensorList.size(); i++) {
			sensorList[i].setParams(aNewParams[i]);
		}
	}
    
	bool isInit() const;
	void clear();
	void initTraffSensors(const std::vector<TraffSensor> &sensors, const TraffAvgParams &avgPar,
						  bool parallel, int sm_data_wakeup);

	// true - averaging on new small interval
	bool processImage(const Frame &frame);
	void toTraffStat(std::vector<TraffStat> &stats, bool updateTime);
	void resetSensorStat(int sens_id);
	int carInChanged(std::vector<TraffStat> &curTraffStats, bool updateTime);
	int counterChanged(std::vector<TraffStat> &curTraffStats, bool updateTime);
    
private:
	TraffCounter(const TraffCounter&);

	TraffCounter& operator=(const TraffCounter&);

	std::vector<TraffSensor> sensorList;

	std::vector<TraffStat> preTraffStats;

	TraffAvgParams avg;

	timeval averagingStartTime;

	int sm_data_wakeup_mode;
	
	std::string lastSmallIntervalTime;
};

}

#endif // OLD_TRAFFCOUNTER_H
