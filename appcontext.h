#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

#include "list.h"

enum {
    REGION_SIZE = 128,
    LINE_SIZE = 1024,
    COLUMN_COUNT = 7,
    STATUS_BAR_MESSAGE_TIMEOUT_MS = 4000,
};

typedef enum {
    OK,
    ERR_FILE_OPEN,
    ERR_INVALID_HEADER,
    ERR_MEMORY,
    ERR_EMPTY_DATA,
    ERR_INVALID_REGION,
    ERR_INVALID_COLUMN
} Status;

typedef struct {
    size_t accepted;
    size_t rejected;
} RowCounts;

typedef struct {
    double min;
    double max;
    double median;
} Metrics;

typedef struct {
    List* list;
    RowCounts parseInfo;
    Status status;
    Metrics metrics;
} AppContext;

#endif // APP_CONTEXT_H

