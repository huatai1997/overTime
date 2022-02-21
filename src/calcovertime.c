#include "calcovertime.h"

#define TIME_AT_MORNING       	  (8 * 3600 + 30 * 60)
#define TIME_AT_NOON              (12 * 3600)
#define TIME_AT_AFTERNOON         (13 * 3600 + 30 * 60)
#define TIME_AT_NIGHT             (18 * 3600)
#define TIME_AT_DELAY_WORK_NIGHT  (18 * 3600 + 30 * 60)
#define TIME_AT_DELAY_WORK        (20 * 3600 + 30 * 60)

/**
 * 非工作日计算加班时长
 * @param timeToWork [in] 上班时间
 * @param timeFromWork [in] 下班时间
 * @return 整数 0=失败 其他表示正常
 */
static int get_over_time_no_work(int timeToWork, int timeFromWork)
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

/**
 * 工作日计算加班时长
 * @param timeToWork [in] 上班时间
 * @param timeFromWork [in] 下班时间
 * @return 整数 0=失败 其他表示正常
 */
static int get_over_time_is_work(int timeToWork, int timeFromWork)
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

/**
 * 从结构体数组中计算出加班时长
 * @param map [in] 结构体数组指针
 * @return 整数 -1=失败 其他表示成功
 */
int get_over_time_from_map(mouthInfo *map)
{
	if (NULL == map)
	{
		return -1;
	}

	int index, totalOverTime = 0;
	for (index = 0; index < 31; index++)
	{
		if (
                map->dayInfoList[index].isSign &&
				map->dayInfoList[index].timeToWork != -1 &&
				map->dayInfoList[index].timeFromWork != -1
           )
		{

			int tmp = map->dayInfoList[index].isWork ? get_over_time_is_work(map->dayInfoList[index].timeToWork, map->dayInfoList[index].timeFromWork) :
													   get_over_time_no_work(map->dayInfoList[index].timeToWork, map->dayInfoList[index].timeFromWork);

			printf("%s : 打卡时间: %02d:%02d:%02d -> %02d:%02d:%02d 加班时长: \033[1;40;33m%02d\033[0m:\033[1;40;33m%02d\033[0m:\033[1;40;33m%02d\033[0m ",
																				map->dayInfoList[index].date,
																				map->dayInfoList[index].timeToWork / 3600,
																				(map->dayInfoList[index].timeToWork % 3600) / 60,
																				map->dayInfoList[index].timeToWork % 60,
																				map->dayInfoList[index].timeFromWork / 3600,
																				(map->dayInfoList[index].timeFromWork % 3600) / 60,
																				map->dayInfoList[index].timeFromWork % 60,
																				tmp / 3600,
																				(tmp % 3600) / 60,
																				tmp % 60);
			if (map->dayInfoList[index].workflow != -1)
			{
				printf("调休时长: \033[1;40;33m%02d\033[0m:\033[1;40;33m%02d\033[0m:\033[1;40;33m%02d\033[0m\n",
														map->dayInfoList[index].workflow / 3600,
														(map->dayInfoList[index].workflow % 3600) / 60,
														map->dayInfoList[index].workflow % 60);

				tmp -= map->dayInfoList[index].workflow;
			}
			else
			{
				printf("调休时长: 0\n");
			}

			totalOverTime += tmp;
		}
	}

	return totalOverTime;
}
