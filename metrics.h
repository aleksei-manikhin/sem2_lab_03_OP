#ifndef METRICS_H
#define METRICS_H

#include "appcontext.h"
#include "demography_record.h"

#ifdef __cplusplus
extern "C" {
#endif

Status calculateMetrics(AppContext* context, const char* region, Column column);

#ifdef __cplusplus
}
#endif

#endif // METRICS_H
