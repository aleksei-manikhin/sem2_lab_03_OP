#include "metrics.h"

#include <string.h>

#include "chart_data.h"
#include "iterator.h"
#include "list.h"

int isDoubleGreater(const void* left, const void* right);
Status validateMetricsInput(const AppContext* context, const char* region, Column column);
Status prepareChartPoints(AppContext* context);
Status insertMetricPoint(List* sortedValues, List* chartPoints, const DemographyRecord* record, Column column);
Status insertRegionData(const AppContext* context, const char* region, Column column, List* sortedValues);
void fillMetricsFromSorted(Metrics* metrics, const List* sortedValues);

int isDoubleGreater(const void* left, const void* right) {
  const double* a = left;
  const double* b = right;

  return *a > *b;
}

Status validateMetricsInput(const AppContext* context, const char* region, Column column) {
  Status status = OK;

  if (context == NULL || context->list == NULL || region == NULL)
    status = ERR_EMPTY_DATA;
  else
    status = validateNumericColumn(column);

  return status;
}

Status prepareChartPoints(AppContext* context) {
  Status status = OK;

  if (context->chartPoints == NULL) {
    context->chartPoints = initList(sizeof(ChartPoint));
    if (context->chartPoints == NULL)
      status = ERR_MEMORY;
  } else
    clearList(context->chartPoints);

  return status;
}

Status insertMetricPoint(List* sortedValues, List* chartPoints, const DemographyRecord* record, Column column) {
  Status status = OK;
  double value = getColumnValue(record, column);

  if (!insertSorted(sortedValues, &value, isDoubleGreater))
    status = ERR_MEMORY;
  else
    status = pushChartPointValueSorted(chartPoints, record->year, value);

  return status;
}

Status insertRegionData(const AppContext* context, const char* region, Column column, List* sortedValues) {
  Status status = OK;
  Iterator it = begin(context->list);

  while (isSet(&it) && status == OK) {
    DemographyRecord* record = get(&it);
    if (record != NULL && !strcmp(record->region, region))
      status = insertMetricPoint(sortedValues, context->chartPoints, record, column);
    next(&it);
  }

  return status;
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
    else
      status = prepareChartPoints(context);
    if (status == OK)
      status = insertRegionData(context, region, column, sortedValues);
    if (status == OK && sortedValues->size == 0)
      status = ERR_INVALID_REGION;
    if (status == OK)
      fillMetricsFromSorted(&context->metrics, sortedValues);
    disposeList(sortedValues);
  }

  if (context != NULL)
    context->status = status;

  return status;
}

