#ifndef FILE_LOADER_H
#define FILE_LOADER_H

#include "list.h"
#include "parser.h"
#include "appcontext.h"

#ifdef __cplusplus
extern "C" {
#endif

Status loadDemographyData(const char* filePath, List* list, RowCounts* info);

#ifdef __cplusplus
}
#endif

#endif // FILE_LOADER_H
