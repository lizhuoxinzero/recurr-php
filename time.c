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

#include "recurr.h"

const int Second = 1;
const int Minute = 60;
const int Hour   = 3600;
//const datetime_t NOFUTURE = ((__int64_t)(1)<<63) - 1;
//const datetime_t NOHISTORY = (__int64_t)(-1) << 63;
const datetime_t NOHISTORY = 0x8000000000000000LL;
const datetime_t NOFUTURE  = 0x7FFFFFFFffffffffLL;

const int secondsPerMinute = 60;
const int secondsPerHour   = 60 * 60;
const int secondsPerDay    = 24 * 3600;
const int secondsPerWeek   = 7 * 86400;
const int daysPer400Years  = 365*400 + 97;
const int daysPer100Years  = 365*100 + 24;
const int daysPer4Years    = 365*4 + 1;

const __int64_t absoluteZeroYear = -292277022399LL;
const int internalYear = 1;
const int unixYear = 1970;
const __int64_t absoluteToInternal = -9223371966579724800LL; //(__int64_t)((absoluteZeroYear - internalYear) * 365.2425) * secondsPerDay; // fix
const __int64_t internalToAbsolute = 9223371966579724800LL; //-absoluteToInternal;
const __int64_t unixToInternal = 62135596800;//(__int64_t)(1969*365 + 1969/4 - 1969/100 + 1969/400) * secondsPerDay;
const __int64_t internalToUnix = -62135596800;//-unixToInternal;

const int daysBefore[] = {
    0,
    31,
    31 + 28,
    31 + 28 + 31,
    31 + 28 + 31 + 30,
    31 + 28 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31,
};

//void output() {
//    printf("--- atoi %ld utoi %ld\n", absoluteToInternal, unixToInternal);
//    printf("--- azy %ld\n", absoluteZeroYear);
//}

void norm(int *hi, int *lo, int base);
int divmod(datetime_t dt, __int64_t d);

char is_leap(int year) {
    return year%4 == 0 && (year%100 != 0 || year%400 == 0);
}

int days_in(int m, int year) {
    if (m == February && is_leap(year)) {
        return 29;
    }
    return daysBefore[m] - daysBefore[m-1];
}

__int64_t dt_abs(datetime_t dt) {
    return dt + internalToAbsolute;
}

__int64_t dt_unix(datetime_t dt) {
    return dt + internalToUnix;
}

datetime_t sdt_unix(__int64_t sec) {
    return sec + unixToInternal;
}

#define IFSET(rval, val)  do{if(rval){*(rval) = (val);}} while(0)

void absdate(__int64_t abs, char full, int *ryear, int *rmonth, int *rday, int *ryday) {
    // Split into time and day.
    __int64_t d = abs / secondsPerDay;
    int year, month, day, yday;

    // Account for 400 year cycles.
    __int64_t n = d / daysPer400Years;
    __int64_t y = 400 * n;
    d -= daysPer400Years * n;

    // Cut off 100-year cycles.
    // The last cycle has one extra leap year, so on the last day
    // of that year, day / daysPer100Years will be 4 instead of 3.
    // Cut it back down to 3 by subtracting n>>2.
    n = d / daysPer100Years;
    n -= n >> 2;
    y += 100 * n;
    d -= daysPer100Years * n;

    // Cut off 4-year cycles.
    // The last cycle has a missing leap year, which does not
    // affect the computation.
    n = d / daysPer4Years;
    y += 4 * n;
    d -= daysPer4Years * n;

    // Cut off years within a 4-year cycle.
    // The last year is a leap year, so on the last day of that year,
    // day / 365 will be 4 instead of 3.  Cut it back down to 3
    // by subtracting n>>2.
    n = d / 365;
    n -= n >> 2;
    y += n;
    d -= 365 * n;

    year = y + absoluteZeroYear;
    yday = d;

    IFSET(ryear, year);
    IFSET(ryday, yday);
    if (!full) {
        return;
    }

    day = yday;
    if (is_leap(year)) {
        // Leap year
        if (day > 31+29-1) {
            // After leap day; pretend it wasn't there.
            day--;
        } else if (day == 31+29-1) {
            // Leap day.
            month = 2;
            day = 29;
            IFSET(rmonth, month);
            IFSET(rday, day);
            return;
        }
    }

    // Estimate month on assumption that every month has 31 days.
    // The estimate may be too low by at most one month, so adjust.
    month = day / 31;
    int end = daysBefore[month+1];
    int begin;
    if (day >= end) {
        month++;
        begin = end;
    } else {
        begin = daysBefore[month];
    }

    month++; // because January is 1
    day = day - begin + 1;

    IFSET(ryear, year);
    IFSET(ryday, yday);
    IFSET(rmonth, month);
    IFSET(rday, day);
}

void dt_date(datetime_t dt, int *year, int *month, int *day) {
    __int64_t abs = dt_abs(dt);
    int yday;
    absdate(abs, 1, year, month, day, &yday);
}


// Weekday returns the day of the week specified by t.
int dt_weekday(datetime_t dt) {
    __int64_t abs = dt_abs(dt);
    __int64_t sec = (abs + Monday*secondsPerDay) % secondsPerWeek;
    return sec / secondsPerDay;
}

void dt_isoweek(datetime_t t, int *ryear, int *rweek) {
    int year, month, day, yday, week;
    __int64_t abs = dt_abs(t);
    absdate(abs, 1, &year, &month, &day, &yday);
    __int64_t wday = (dt_weekday(t)+6) % 7; // weekday but Monday = 0.
    enum { Mon = 0, Tue, Wed, Thu, Fri, Sat, Sun, };

    week = (yday - wday + 7) / 7;

    __int64_t jan1wday = (wday - yday + 7*53) % 7;
    if (Tue <= jan1wday && jan1wday <= Thu) {
        week++;
    }

    // If the week number is still 0, we're in early January but in
    // the last week of last year.
    if (week == 0) {
        year--;
        week = 52;
        // A year has 53 weeks when Jan 1 or Dec 31 is a Thursday,
        // meaning Jan 1 of the next year is a Friday
        // or it was a leap year and Jan 1 of the next year is a Saturday.
        if (jan1wday == Fri || (jan1wday == Sat && is_leap(year))) {
            week++;
        }
    }

    // December 29 to 31 are in week 1 of next year if
    // they are after the last Thursday of the year and
    // December 31 is a Monday, Tuesday, or Wednesday.
    if (month == December && day >= 29 && wday < Thu) {
        __int64_t dec31wday = (wday + 31 - day) % 7;
        if (Mon <= dec31wday && dec31wday <= Wed) {
            year++;
            week = 1;
        }
    }

    IFSET(ryear, year);
    IFSET(rweek, week);
}

int dt_hour(datetime_t dt) {
    return dt_abs(dt)%secondsPerDay / secondsPerHour;
}

int dt_minute(datetime_t dt) {
    return dt_abs(dt)%secondsPerHour / secondsPerMinute;
}

int dt_second(datetime_t dt) {
    return dt_abs(dt) % secondsPerMinute;
}

int dt_yearday(datetime_t dt) {
    int year, month, day, yday;
    __int64_t abs = dt_abs(dt);
    absdate(abs, 0, &year, &month, &day, &yday);
    return yday + 1;
}

datetime_t sdt_datefix(int year, int month, int day, int mfix) {
    // Normalize month, overflowing into year.
    int m = month - 1;
    norm(&year, &m, 12);
    month = m + 1;

    if (mfix > Ignore && day > 28) {
        int nmonth = days_in(month, year);
        if (day > nmonth) {
            switch (mfix) {
            case UseLastDay:
                day = nmonth;
                break;
            }
        }
    }

    __int64_t y = year - absoluteZeroYear;

    __int64_t n = y / 400;
    y -= 400 * n;
    __int64_t d = daysPer400Years * n;

    // Add in 100-year cycles.
    n = y / 100;
    y -= 100 * n;
    d += daysPer100Years * n;

    // Add in 4-year cycles.
    n = y / 4;
    y -= 4 * n;
    d += daysPer4Years * n;

    // Add in non-leap years.
    n = y;
    d += 365 * n;

    // Add in days before this month.
    d += daysBefore[month-1];
    if (is_leap(year) && month >= March) {
        d++; // February 29
    }

    // Add in days before today.
    d += day - 1;

    // Add in time elapsed today.
    __int64_t abs = d * secondsPerDay;

    return abs + absoluteToInternal;
}

datetime_t dt_truncate_day(datetime_t t) {
    return t - t % secondsPerDay;
}

datetime_t dt_truncate(datetime_t t, __int64_t d) {
    if (d <= 0) {
        return t;
    }
    __int64_t r = divmod(t, d);
    return t - r;
}

datetime_t dt_round(datetime_t t, __int64_t d) {
    if (d <= 0) {
        return t;
    }
    __int64_t r = divmod(t, d);
    if (r+r < d) {
        return t-r;
    }
    return t + (d - r);
}

void norm(int *hi, int *lo, int base) {
    int n;
    if (*lo < 0) {
        n = (-*lo - 1)/base + 1;
        *hi -= n;
        *lo += n * base;
    }
    if (*lo >= base) {
        n = *lo / base;
        *hi += n;
        *lo -= n * base;
    }
}

int divmod(datetime_t dt, __int64_t d) { char neg = 0;
    if (dt < 0) {
        neg = 1;
        dt = -dt;
    }
    __int64_t r;
    r = dt % d;
    if (neg && r != 0) {
        r = d - r;
    }
    return r;
}

char tc_sorted(timeclude_t *tc) {
    int i;
    for (i = 1; i < tc->size; i++) {
        if (tc->dates[i-1] > tc->dates[i]) {
            return 0;
        }
    }
    return 1;
}

char tc_isexclude(timeclude_t *tc, datetime_t dt) {
    if (tc->byday) {
        dt = dt_truncate_day(dt);
    }
    while (tc->pos < tc->size) {
        if (tc->dates[tc->pos] < dt) {
            ++tc->pos;
        } else if (tc->dates[tc->pos] > dt) {
            return 0;
        } else {
            return 1;
        }
    }
    return 0;
}


void tc_reset(timeclude_t *tc, datetime_t dt) {
    if (tc->byday) {
        dt = dt_truncate_day(dt);
    }
    int i, j, h;
    i = 0; j = tc->size;
    while (i < j) {
        h = i + (j-i)/2;
        if (tc->dates[i] < dt) {
            i = h + 1; // preserves f(i-1) == 0
        } else {
            j = h;
        }
    }
    tc->pos = i;
}


