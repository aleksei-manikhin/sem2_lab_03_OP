#include "entrypoint.h"

#include "logic.h"

void doOperation(Operation operation, AppContext* context, const AppParams* params) {
  switch (operation) {
    case INITIALIZE:
      initContext(context);
      break;

    case LOAD_DATA:
      loadData(context, params->str);
      break;

    case CALCULATE_AND_DRAW:
      calculateMetricsForRegion(context, params->str, params->column);
      break;

    case DISPOSE:
      disposeContext(context);
      break;
  }
}
