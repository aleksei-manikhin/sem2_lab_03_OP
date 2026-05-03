#include "logic.h"

#include "file_loader.h"
#include "metrics.h"

void clearParseInfo(RowCounts* parseInfo);
void clearMetrics(Metrics* metrics);
Status prepareListForLoad(AppContext* context);
Status loadAndCheckData(AppContext* context, const char* filePath);

void clearParseInfo(RowCounts* parseInfo) {
  if (parseInfo != NULL) {
    parseInfo->accepted = 0;
    parseInfo->rejected = 0;
  }
}

void clearMetrics(Metrics* metrics) {
  if (metrics != NULL) {
    metrics->min = 0.0;
    metrics->max = 0.0;
    metrics->median = 0.0;
  }
}

Status prepareListForLoad(AppContext* context) {
  Status status = OK;
  if (context->list == NULL){
    if (!(context->list = initList(sizeof(DemographyRecord))))
      status = ERR_MEMORY;
  }else
    clearList(context->list);
  return status;
}


Status loadAndCheckData(AppContext* context, const char* filePath) {
  Status status = loadDemographyData(filePath, context->list, &context->parseInfo);
  return (status == OK && context->list->size == 0) ? ERR_EMPTY_DATA : status;
}


void initContext(AppContext* context) {
  if (context != NULL) {
    context->list = NULL;
    clearParseInfo(&context->parseInfo);
    clearMetrics(&context->metrics);
    context->status = OK;
  }
}

void disposeContext(AppContext* context) {
  if (context != NULL) {
    if (context->list != NULL) {
      disposeList(context->list);
      context->list = NULL;
    }
    clearParseInfo(&context->parseInfo);
    clearMetrics(&context->metrics);
    context->status = OK;
  }
}

Status loadData(AppContext* context, const char* filePath) {
  Status status = OK;

  if (context == NULL)
    status = ERR_EMPTY_DATA;
  else if (filePath == NULL)
    status = ERR_FILE_OPEN;
  else {
    status = prepareListForLoad(context);
    clearParseInfo(&context->parseInfo);
    clearMetrics(&context->metrics);
    if (status == OK)
      status = loadAndCheckData(context, filePath);

    context->status = status;
  }
  return status;
}

Status calculateMetricsForRegion(AppContext* context, const char* region, Column column) {
  Status status = OK;

  if (context == NULL)
    status = ERR_EMPTY_DATA;
  else {
    status = calculateMetrics(context, region, column);
    context->status = status;
  }
  return status;
}

