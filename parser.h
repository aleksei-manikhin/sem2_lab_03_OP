#ifndef PARSER_H
#define PARSER_H

#include "demography_record.h"
#include "appcontext.h"

#ifdef __cplusplus
extern "C" {
#endif

int validateCsvHeader(const char* headerLine);
int parseDemographyLine(const char* line, DemographyRecord* record);

#ifdef __cplusplus
}
#endif

#endif // PARSER_H
