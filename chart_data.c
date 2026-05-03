#include "chart_data.h"

int isChartPointYearGreater(const void* left, const void* right);

Status validateNumericColumn(Column column) {
  Status status = OK;
  if (column < YEAR || column > URBANIZATION || column == REGION)
    status = ERR_INVALID_COLUMN;
  return status;
}

double getColumnValue(const DemographyRecord* record, Column column) {
  double value = 0.0;

  if (record != NULL) {
    switch (column) {
      case YEAR:
        value = record->year;
        break;
      case NPG:
        value = record->naturalPopulationGrowth;
        break;
      case BIRTH_RATE:
        value = record->birthRate;
        break;
      case DEATH_RATE:
        value = record->deathRate;
        break;
      case GDW:
        value = record->generalDemographicWeight;
        break;
      case URBANIZATION:
        value = record->urbanization;
        break;
      default:
        break;
    }
  }
  return value;
}

int isChartPointYearGreater(const void* left, const void* right) {
  const ChartPoint* a = left;
  const ChartPoint* b = right;

  return a->year > b->year;
}

Status pushChartPointValueSorted(List* points, int year, double value) {
  ChartPoint point;
  Status status = OK;

  if (points == NULL)
    status = ERR_EMPTY_DATA;
  else {
    point.year = year;
    point.value = value;
    if (!insertSorted(points, &point, isChartPointYearGreater))
      status = ERR_MEMORY;
  }

  return status;
}
