#ifndef _MYTIME_H_
#define _MYTIME_H_

typedef struct dateNode {
	unsigned int year;
	unsigned int month;
	unsigned int day;
}date_t;

void get_the_current_date(date_t *date);
int get_wday_by_date(int year, int month, int day);
int is_leap_year(int year);

#endif
