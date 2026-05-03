#include "parser.h"

#include <stdio.h>
#include <string.h>

int validateCsvHeader(const char* headerLine) {
  return headerLine != NULL &&
         strstr(headerLine, "year") != NULL &&
         strstr(headerLine, "region") != NULL &&
         strstr(headerLine, "birth_rate") != NULL &&
         strstr(headerLine, "death_rate") != NULL &&
         strstr(headerLine, "urbanization") != NULL;
}

int parseDemographyLine(const char* line, DemographyRecord* record) {
  return line != NULL &&
        record != NULL &&
        sscanf(line, "%d,%127[^,],%lf,%lf,%lf,%lf,%lf",
                           &record->year,
                           record->region,
                           &record->naturalPopulationGrowth,
                           &record->birthRate,
                           &record->deathRate,
                           &record->generalDemographicWeight,
                           &record->urbanization) == COLUMN_COUNT;
}
