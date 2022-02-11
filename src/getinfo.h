#ifndef _GETINFO_H_
#define _GETINFO_H_

#include "common.h"


typedef struct {
	bool isSign;
    bool isWork;
    unsigned int timeToWork;
    unsigned int timeFromWork;
	unsigned int workflow;
	char date[12];
}dayInfo;

typedef struct {
    dayInfo dayInfoList[31];
}mouthInfo;

int save_work_info_to_map(const char *file, mouthInfo *map);

#endif
