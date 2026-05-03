#ifndef DEMOGRAPHY_RECORD_H
#define DEMOGRAPHY_RECORD_H

#include "appcontext.h"

typedef enum {
    YEAR,
    REGION,
    NPG,
    BIRTH_RATE,
    DEATH_RATE,
    GDW,
    URBANIZATION,
} Column;


typedef struct
{
    int year;
    char region[REGION_SIZE];
    double naturalPopulationGrowth;
    double birthRate;
    double deathRate;
    double generalDemographicWeight;
    double urbanization;
} DemographyRecord;

#endif // DEMOGRAPHY_RECORD_H

