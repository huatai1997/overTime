#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mytime.h"

#define CURRENT_WORK_TIME_PATH    "./current_path"
#define TIME_AT_MORNING    	  (8 * 3600 + 30 * 60)
#define TIME_AT_NOON       	  (12 * 3600)
#define TIME_AT_AFTERNOON  	  (13 * 3600 + 30 * 60)
#define TIME_AT_NIGHT      	  (18 * 3600)
#define TIME_AT_DELAY_WORK_NIGHT  (18 * 3600 + 30 * 60)
#define TIME_AT_DELAY_WORK        (20 * 3600 + 30 * 60)
#define FALSE 1
#define TRUE  0

// 目的加班时长, 缺省为60h
static int sg_i_target_overwork_time = 0;

// 调休时长
static int sg_i_paid_leave_time = 0;

typedef struct{
	int isWeek;
	int timeToWork;
	int timeFromWork;
}workTime;

typedef struct {
	int workDays;
	workTime workTimeList[31];
}workTimeMap;

static date_t now_time;
static int leap_year_days[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static int no_leap_year_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static date_t wdayList[31];
static date_t rdayList[31];

static char *get_current_path(void)
{
	FILE *fp = NULL;
	static char path[128] = "";

	fp = fopen(CURRENT_WORK_TIME_PATH, "r");
	if (NULL == fp)
	{
		return NULL;
	}

	if (NULL == fgets(path, sizeof(path), fp))
	{
		fclose(fp);
		fp = NULL;
		return NULL;
	}

	fclose(fp);
	fp = NULL;

	return (char *)path;
}

static int write_path_to_file(char *path)
{
	if (NULL == path)
	{
		return FALSE;
	}

	FILE *fp = fopen(CURRENT_WORK_TIME_PATH, "w+");
	if (fp != NULL)
	{
		fwrite(path, strlen(path), sizeof(char), fp);
		fclose(fp);
		fp = NULL;
		return TRUE;
	}

	return FALSE;
}

static int get_work_time_from_file(workTimeMap *wtl, char *file)
{
	if (NULL == wtl)
	{
		return FALSE;
	}

	FILE *fp = NULL;
	char *path = NULL;
	char line[256] = "";
	int index = 0;
	int isWeekend = 0;
	int ttk = 0, tfw = 0;
	int ttwHours = 0, ttwMinutes = 0, ttwSeconds = 0;
	int tfwHours = 0, tfwMinutes = 0, tfwSeconds = 0;

	if (file != NULL)
	{
		path = file;
		write_path_to_file(file);
	}
	else
	{
		path = get_current_path();
	}

	if (NULL == path)
	{
		return FALSE;
	}

	fp = fopen(path, "r");
	if (NULL == fp)
	{
		return FALSE;
	}

	while (fgets(line, 256, fp) != NULL)
	{
		sscanf(line, "%d,%d,%d", &ttk, &tfw, &isWeekend);

		ttwSeconds = ttk % 100;
		ttwMinutes = ttk / 100 % 100;
		ttwHours   = ttk / 10000;

		tfwSeconds = tfw % 100;
		tfwMinutes = tfw / 100 % 100;
		tfwHours   = tfw / 10000;
		
		wtl->workTimeList[index].isWeek         = isWeekend;
		wtl->workTimeList[index].timeToWork     = ttwHours * 3600 + ttwMinutes * 60 + ttwSeconds;
		wtl->workTimeList[index++].timeFromWork = tfwHours * 3600 + tfwMinutes * 60 + tfwSeconds;
	}

	fclose(fp);
	wtl->workDays = index;

	return TRUE;
}

static int get_over_time_is_week(int timeToWork, int timeFromWork)
{
	if (timeFromWork <= timeToWork)
	{
		return 0;
	}

	int timeForDay = 0;

	if (timeToWork <= TIME_AT_NOON)
	{
		if (timeFromWork <= TIME_AT_AFTERNOON) // 13:30前走的
                {
                        timeForDay = TIME_AT_NOON - timeToWork;
                }
                else  // 13:30后走的
                {
                        timeForDay = TIME_AT_NOON - timeToWork + timeFromWork - TIME_AT_AFTERNOON;
                }
	}
	else if (timeToWork <= TIME_AT_AFTERNOON)
	{
		timeForDay = timeFromWork - TIME_AT_AFTERNOON;
	}
	else if (timeToWork > TIME_AT_AFTERNOON)
	{
		timeForDay = timeFromWork - timeToWork;
	}

	if (timeFromWork >= (19 * 3600 + 30 * 60))
	{
		timeForDay -= (30 * 60);
	}

	return timeForDay > 0 ? timeForDay : 0;	
}

static int get_over_time_no_week(int timeToWork, int timeFromWork)
{
	if (timeFromWork <= timeToWork)
        {
                return 0;
        }

	int timeForDay = 0;

	if (timeFromWork >= TIME_AT_DELAY_WORK) // 延迟下班
        {
                timeForDay = (timeFromWork - TIME_AT_DELAY_WORK_NIGHT) - (timeToWork > TIME_AT_MORNING ? timeToWork - TIME_AT_MORNING : 0);
        }
        else
        {
                timeForDay = (timeFromWork - TIME_AT_NIGHT) - (timeToWork > TIME_AT_MORNING ? timeToWork - TIME_AT_MORNING : 0);
        }

	return timeForDay > 0 ? timeForDay : 0;
}

static void get_workingDays_and_restDays(int lastDay, int *wday, int *rday)
{
	int isLeapYear = is_leap_year(now_time.year);
	int totalDays = isLeapYear ? leap_year_days[now_time.month-1] : no_leap_year_days[now_time.month-1];

	int i = 0;
	int tmpWday = 0, tmpRday = 0;
	int month = lastDay / 100;
	int day = lastDay % 100 + 1;

	for (i = day; i <= totalDays; i++)
	{
		int flag = get_wday_by_date(now_time.year, month, i);
		if (flag != 5 && flag != 0)
		{
			if (6 == flag) // 周六
			{
				rdayList[tmpRday].year = now_time.year;
				rdayList[tmpRday].month = month;
				rdayList[tmpRday].day = i;
				tmpRday++;
			}
			else // 工作日
			{
				wdayList[tmpWday].year = now_time.year;
                wdayList[tmpWday].month = month;
                wdayList[tmpWday].day = i;
                tmpWday++;
			}
		}
	}

	*wday = tmpWday;
	*rday = tmpRday;
}

static void get_advice_for_future(int overTime, int type)
{
	int i = 0;
	int workingDays = 0, restDays = 0;
	int workingDaysStartTime = 0, restDaysStartTime = 0;
	int workingDaysWorkTime = 0, restDaysWorkTime = 0;
	int differenceValue = sg_i_target_overwork_time - overTime;

	get_workingDays_and_restDays(type, &workingDays, &restDays);

	printf("\tYou need \033[1;40;33m%d\033[0m hours \033[1;40;33m%d\033[0m minutes then you will have \033[1;40;33m%d\033[0m hours overtime.\n\n\tHere are some tips for you,\n\n",
				differenceValue / 3600, differenceValue % 3600 / 60, sg_i_target_overwork_time / 3600);

	if (0 == restDays)
	{
		restDaysWorkTime = 0;
		workingDaysWorkTime = differenceValue / workingDays;
		printf("\tYou can work from \033[1;40;33m08:30\033[0m to \033[1;40;33m%02d:%02d\033[0m in the following days:\n\n", (TIME_AT_DELAY_WORK_NIGHT + workingDaysWorkTime) / 3600, 
									       ((TIME_AT_DELAY_WORK_NIGHT + workingDaysWorkTime) % 3600) / 60);

		printf("\t");
		for (i = 0; i < workingDays; i++)
		{
			printf("%d-%02d-%02d  ", wdayList[i].year, wdayList[i].month, wdayList[i].day);
			if (i != 0 && i % 4 == 0)
			{
				printf("\n\t");
			}
		}
		printf("\n\n");
	}
	else if (0 == workingDays)
	{
		workingDaysWorkTime = 0;
		restDaysWorkTime = differenceValue / restDays;
		printf("\tYou can work from \033[1;40;33m08:30\033[0m to \033[1;40;33m%02d:%02d\033[0m in the following days:\n\n",
			(TIME_AT_MORNING + restDaysWorkTime) > TIME_AT_NOON ? ((TIME_AT_MORNING + restDaysWorkTime + (3600 + 60 * 30)) / 3600) : ((TIME_AT_MORNING + restDaysWorkTime) / 3600),
			(TIME_AT_MORNING + restDaysWorkTime) > TIME_AT_NOON ? ((TIME_AT_MORNING + restDaysWorkTime + (3600 + 60 * 30)) % 3600 / 60) : ((TIME_AT_MORNING + restDaysWorkTime) % 3600 / 60));

		printf("\t");
		for (i = 0; i < restDays; i++)
		{
			printf("%d-%02d-%02d  ", rdayList[i].year, rdayList[i].month, rdayList[i].day);
			if (i != 0 && i % 4 == 0)
			{
				printf("\n\t");
			}
		}
		printf("\n\n");
	}
	else
	{
		restDaysWorkTime = (differenceValue * 9) / (5 * workingDays + 9 * restDays);
		workingDaysWorkTime = (restDaysWorkTime * 5) / 9;
		printf("\tYou can work from \033[1;40;33m08:30\033[0m to \033[1;40;33m%02d:%02d\033[0m in the following days:\n\n",
			(TIME_AT_DELAY_WORK_NIGHT + workingDaysWorkTime) / 3600, ((TIME_AT_DELAY_WORK_NIGHT + workingDaysWorkTime) % 3600) / 60);

		printf("\t");
		for (i = 0; i < workingDays; i++)
		{
			printf("%d-%02d-%02d  ", wdayList[i].year, wdayList[i].month, wdayList[i].day);
			if (i != 0 && i % 4 == 0)
			{
				printf("\n\t");
			}
		}
		printf("\n");

		printf("\twork form \033[1;40;33m08:30\033[0m to \033[1;40;33m%02d:%02d\033[0m in the following days:\n\n",
			(TIME_AT_MORNING + restDaysWorkTime) > TIME_AT_NOON ? ((TIME_AT_MORNING + restDaysWorkTime + (3600 + 60 * 30)) / 3600) : ((TIME_AT_MORNING + restDaysWorkTime) / 3600),
			(TIME_AT_MORNING + restDaysWorkTime) > TIME_AT_NOON ? ((TIME_AT_MORNING + restDaysWorkTime + (3600 + 60 * 30)) % 3600 / 60) : ((TIME_AT_MORNING + restDaysWorkTime) % 3600 / 60));

		printf("\t");
		for (i = 0; i < restDays; i++)
		{
			printf("%d-%02d-%02d  ", rdayList[i].year, rdayList[i].month, rdayList[i].day);
			if (i != 0 && i % 4 == 0)
			{
				printf("\n\t");
			}
		}
		printf("\n\n");

	}
	printf("\tBest wish!\n\n");
}

static int get_over_time_from_map(workTimeMap *wtl, int future)
{
	if (NULL == wtl)
	{
                return FALSE;
    }

	int i = 0;
	int overTimeofDay = 0;
	int overTimeOfMonth = 0;

	for (i = 0; i < wtl->workDays; i++)
	{
		overTimeofDay = wtl->workTimeList[i].isWeek == 1 ? get_over_time_is_week(wtl->workTimeList[i].timeToWork, wtl->workTimeList[i].timeFromWork) 
								 : get_over_time_no_week(wtl->workTimeList[i].timeToWork, wtl->workTimeList[i].timeFromWork);

		printf(" %02d:%02d -> %02d:%02d  :  %02d:%02d\n", wtl->workTimeList[i].timeToWork / 3600, (wtl->workTimeList[i].timeToWork % 3600) / 60,
						    wtl->workTimeList[i].timeFromWork / 3600, (wtl->workTimeList[i].timeFromWork % 3600) / 60,
						    overTimeofDay / 3600, (overTimeofDay % 3600) / 60);
		overTimeOfMonth += overTimeofDay;
	}

	// 减去调休时长
	overTimeOfMonth -= sg_i_paid_leave_time;

	printf("\nHi:\n\tYou had worked \033[1;40;33m%d\033[0m hours \033[1;40;33m%d\033[0m minutes \033[1;40;33m%d\033[0m seconds of overtime this month!\n\n",
				 overTimeOfMonth / 3600, (overTimeOfMonth % 3600) / 60, overTimeOfMonth % 60);

	if (sg_i_paid_leave_time > 0)
	{
		printf("\tYou had paided leave \033[1;40;33m%d\033[0m hours this month!\n\n", sg_i_paid_leave_time / 3600);
	}

	if (future != -1)
	{
		overTimeOfMonth < sg_i_target_overwork_time ? get_advice_for_future(overTimeOfMonth, future)
					:printf("\tYou had worked more than \033[1;40;33m%d\033[0m hours of overtime this month!\n\n", sg_i_target_overwork_time / 3600);
	}

	return TRUE;
}

static void get_over_time(int future, char *file)
{
	workTimeMap wtl;
	memset(&wtl, 0, sizeof(workTimeMap));

	get_the_current_date(&now_time);

	if (FALSE == get_work_time_from_file(&wtl, file))
	{
		printf("get_work_time_from_file error, exit!\n");
		return;
	}

	if (FALSE == get_over_time_from_map(&wtl, future))
	{
		printf("get_over_time_from_map error, exit!\n");
		return;
	}
}

static void usage(void)
{
	int ret = system("cat ./readMe");
	exit(-1);
}

int main(int argc, char *argv[])
{
	if (argc > 4 || argc < 2)
	{
		usage();
	}

	if (0 == strcmp(argv[1], "get_time") && argc != 4)
	{
		if (argv[2] != NULL)
		{
			sg_i_paid_leave_time = atoi(argv[2]) * 60 * 60;
		}

		get_over_time(-1, NULL);
	}
	else if (0 == strcmp(argv[1], "get_advice") && argc != 2)
	{
		sg_i_target_overwork_time = argv[3] != NULL ? atoi(argv[3]) * 60 * 60 : 60 * 60 * 60;

        get_over_time(atoi(argv[2]), NULL);
	}
	else if (0 == strcmp(argv[1], "set_file") && 3 == argc)
	{
		get_over_time(-1, argv[2]);
	}
	else
	{
		usage();
	}

	return 0;
}
