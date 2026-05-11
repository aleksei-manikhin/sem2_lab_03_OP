#ifndef CHART_DATA_H
#define CHART_DATA_H

#include "demography_record.h"
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int year;
    double value;
} ChartPoint;

Status validateNumericColumn(Column column);
double getColumnValue(const DemographyRecord* record, Column column);
Status pushChartPointValueSorted(List* points, int year, double value);

#ifdef __cplusplus
}
#endif

#endif // CHART_DATA_H
