#include "mytime.h"

#include <string.h>
#include <time.h>

void get_the_current_date(date_t *date)
{
	time_t now;
	struct tm *tm_now = NULL;

	time(&now);	
	tm_now = localtime(&now);

	date->year = tm_now->tm_year + 1900;
	date->month = tm_now->tm_mon + 1;
	date->day = tm_now->tm_mday;
}

int get_wday_by_date(int year, int month, int day)
{
	int weekDay = -1;
	if (1 == month || 2 == month)
	{
		month += 12;
		year--;
	}

	/* 0 -> sunday */
	weekDay = (day + 1 + 2 * month + 3 * (month + 1) / 5 + year + year / 4 - year / 100 + year / 400) % 7;

    return weekDay;
}

int is_leap_year(int year)
{
	return ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) ? 1 : 0;
}
