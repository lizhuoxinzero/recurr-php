/**********************************************************\
|                                                          |
| php_recurr.c                                             |
|                                                          |
| RECURR for pecl source file.                             |
|                                                          |
| Code Author:  Never Lee <listarmb@gmail.com>             |
| LastModified: Apr 09, 2015                               |
|                                                          |
\**********************************************************/

#ifndef __RRECURR_H
#define __RRECURR_H

#include <stdio.h>

enum Weekday {
	Sunday = 0,
	Monday,
	Tuesday,
	Wednesday,
	Thursday,
	Friday,
	Saturday,
};

enum Month {
	January = 1,
	February,
	March,
	April,
	May,
	June,
	July,
	August,
	September,
	October,
	November,
	December,
};

typedef __int64_t datetime_t;

char is_leap(int year);
int days_in(int m, int year);
__int64_t dt_unix(datetime_t dt);
__int64_t dt_abs(datetime_t dt);
__int64_t dt_unix(datetime_t dt);
datetime_t sdt_unix(__int64_t sec);
void dt_date(datetime_t dt, int *year, int *month, int *day);
int dt_weekday(datetime_t dt);
void dt_isoweek(datetime_t t, int *ryear, int *rweek);
int dt_hour(datetime_t dt);
int dt_minute(datetime_t dt);
int dt_second(datetime_t dt);
int dt_yearday(datetime_t dt);
datetime_t sdt_datefix(int year, int month, int day, int mfix);
datetime_t dt_truncate(datetime_t t, __int64_t d);
datetime_t dt_round(datetime_t t, __int64_t d);
datetime_t dt_truncate_day(datetime_t t);

enum {
    Ignore = 0,
    //NoThisSchedule
    UseLastDay,
};
enum {
    Solar = 0,
    Lunar,
};

typedef enum {
    SECONDLY = 0,
    MINUTELY,
    HOURLY,
    DAILY,
    WEEKLY,
    MONTHLY,
    YEARLY,
} freq_t;

typedef struct {
    datetime_t *dates;
    int size;
    int pos;
    char byday;
} timeclude_t;
struct rule_s {
    datetime_t  start;
    datetime_t  end;
    freq_t      freq;
    int         interval;
    datetime_t  until;
    timeclude_t exdates;
    int         monthfix;
    int         lunarflag;
};
typedef struct rule_s rule_t;

char tc_sorted(timeclude_t *tc);
char tc_isexclude(timeclude_t *tc, datetime_t dt);
void tc_reset(timeclude_t *tc, datetime_t dt);

#endif// __RRECURR_H
