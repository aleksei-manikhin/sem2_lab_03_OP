#ifndef ENTRYPOINT_H
#define ENTRYPOINT_H

#include "appcontext.h"
#include "demography_record.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  INITIALIZE,
  LOAD_DATA,
  CALCULATE_METRICS,
  DISPOSE
} Operation;

typedef struct {
  const char* str;
  Column column;
} AppParams;

void doOperation(Operation operation, AppContext* context, const AppParams* params);

#ifdef __cplusplus
}
#endif

#endif // ENTRYPOINT_H
