#include "metrics.h"

#include <string.h>

#include "iterator.h"
#include "list.h"

int isNumericColumn(Column column);
int compareDoubleValues(const void* left, const void* right);
Status validateMetricsInput(const AppContext* context, const char* region, Column column);
double getColumnValue(const DemographyRecord* record, Column column);
int insertRegionValues(const AppContext* context, const char* region, Column column, List* sortedValues);
void fillMetricsFromSorted(Metrics* metrics, const List* sortedValues);

int isNumericColumn(Column column) {
  int isValid = 0;
  if (column >= YEAR && column <= URBANIZATION && column != REGION)
    isValid = 1;

  return isValid;
}

int compareDoubleValues(const void* left, const void* right) {
  int result = 0;
  double a = *(const double*)left;
  double b = *(const double*)right;

  if (a < b)
    result = -1;
  else if (a > b)
    result = 1;

  return result;
}

Status validateMetricsInput(const AppContext* context, const char* region, Column column) {
  Status status = OK;

  if (context == NULL || context->list == NULL || region == NULL)
    status = ERR_EMPTY_DATA;
  else if (!isNumericColumn(column))
    status = ERR_INVALID_COLUMN;

  return status;
}

double getColumnValue(const DemographyRecord* record, Column column) {
  double value = 0.0;

  if (record != NULL) {
    switch (column) {
      case YEAR:
        value = (double)record->year; //
        break;
      case NPG: value = record->naturalPopulationGrowth;
      break;
      case BIRTH_RATE: value = record->birthRate;
      break;
      case DEATH_RATE: value = record->deathRate;
      break;
      case GDW: value = record->generalDemographicWeight;
      break;
      case URBANIZATION: value = record->urbanization;
      break;
      default:
      break;
    }
  }

  return value;
}

int insertRegionValues(const AppContext* context, const char* region, Column column, List* sortedValues) {
  int isSuccess = 1;
  Iterator it = begin(context->list);

  while (isSet(&it) && isSuccess) {
    DemographyRecord* record = (DemographyRecord*)get(&it);
    if (record != NULL && !strcmp(record->region, region)) {
      double value = getColumnValue(record, column);
      if (!insertSorted(sortedValues, &value, compareDoubleValues))
        isSuccess = 0;
    }
    next(&it);
  }

  return isSuccess;
}

void fillMetricsFromSorted(Metrics* metrics, const List* sortedValues) {
  size_t middleIndex = sortedValues->size / 2;
  size_t index = 0;
  double prevValue = 0.0;
  double currentValue = 0.0;
  Iterator it = begin(sortedValues);

  metrics->min = *(double*)sortedValues->head->data;
  metrics->max = *(double*)sortedValues->tail->data;

  while (isSet(&it) && index <= middleIndex) {
    prevValue = currentValue;
    currentValue = *(double*)get(&it);
    index++;
    next(&it);
  }

  if (sortedValues->size % 2 == 0)
    metrics->median = (prevValue + currentValue) / 2.0;
  else
    metrics->median = currentValue;
}

Status calculateMetrics(AppContext* context, const char* region, Column column) {
  Status status = validateMetricsInput(context, region, column);
  List* sortedValues = NULL;

  if (status == OK) {
    sortedValues = initList(sizeof(double));
    if (sortedValues == NULL)
      status = ERR_MEMORY;
    else if (!insertRegionValues(context, region, column, sortedValues))
      status = ERR_MEMORY;
    else if (sortedValues->size == 0)
      status = ERR_INVALID_REGION;
    else
      fillMetricsFromSorted(&context->metrics, sortedValues);
    disposeList(sortedValues);
  }

  context->status = status;

  return status;
}

