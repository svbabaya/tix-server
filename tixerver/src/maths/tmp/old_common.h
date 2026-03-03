#ifndef OLD_COMMON_H
#define OLD_COMMON_H

namespace old {

#include <stdio.h>
#include <syslog.h>

// #define TraffiXtream

#define COMPENSATE_LIGHTCHANGE 1

// #define TraffiX_TRACKER // For arm

#define APP_ID 50001

#if defined(TraffiX_TRACKER)
	#define MAJOR_VERSION	2
	#define MINOR_VERSION	0
	#define VERSION_SUFFIX	"TRACKER"
#else
	#define MAJOR_VERSION	1
	#define MINOR_VERSION	0
	#define VERSION_SUFFIX	"ORDINARY"
#endif

#define GLOBAL_UPDATE_STAT_FRAMENUM		1  // Global statistics update interval (frames)
#define GLOBAL_UPDATE_CONFIG_FRAMENUM	10 // Configuration check

#define LOGINFO(fmt, ...) { syslog(LOG_INFO, fmt, ##__VA_ARGS__); printf(fmt, ##__VA_ARGS__); }
#define LOGERR(fmt, ...) { syslog(LOG_CRIT, fmt, ##__VA_ARGS__); fprintf(stderr, fmt, ##__VA_ARGS__); }

}

#endif // OLD_COMMON_H
