#ifndef DATEMAP_H
#define DATEMAP_H

#include <stdio.h>



typedef struct{
    int year;
    int month;
    int day;
    int isleap;
}lunar_data_t;

typedef struct{
    int year;
    int month;
    int day;
}solar_data_t;

int lunar_to_solar(int year, int month, int day, int isleap, solar_data_t *pSolarData);
int solar_to_lunar(int year, int month, int day, lunar_data_t *pLunarData);


__int64_t lunar_to_stamp(int year, int month, int day, int isleap);

typedef struct{
    __int64_t stamp;
    __int64_t leapstamp;
}triger_result_t;

__int64_t add_lunaryear(int year, int month, int day, int addyear_cnt, triger_result_t *triger_result);

__int64_t add_lunarmonth(int year, int month, int day, int addmonth_cnt);

#endif
