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
int solar_to_lunar(int year, int month, int dayi, lunar_data_t *pLunarData);

#endif
