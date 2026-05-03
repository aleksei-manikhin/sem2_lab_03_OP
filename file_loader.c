#include "file_loader.h"

#include <stdio.h>

Status processLines(FILE* file, List* list, RowCounts* info);

Status loadDemographyData(const char* filePath, List* list, RowCounts* info) {
  Status error = OK;
  FILE* file = NULL;
  char buffer[LINE_SIZE];

  if (filePath == NULL || list == NULL || info == NULL) {
    error = ERR_FILE_OPEN;
  } else {
    info->accepted = 0;
    info->rejected = 0;

    file = fopen(filePath, "r");

    if (file == NULL) {
      error = ERR_FILE_OPEN;
    } else {
      if (fgets(buffer, sizeof(buffer), file) == NULL)
        error = ERR_EMPTY_DATA;
      else if (!validateCsvHeader(buffer))
        error = ERR_INVALID_HEADER;
      else
        error = processLines(file, list, info);

      fclose(file);
    }
  }

  return error;
}

Status processLines(FILE* file, List* list, RowCounts* info) {
  Status error = OK;
  char buffer[LINE_SIZE];
  DemographyRecord record;

  while (fgets(buffer, sizeof(buffer), file) != NULL) {
    if (parseDemographyLine(buffer, &record)) {
      if (pushBack(list, &record)) {
        info->accepted++;
      } else {
        error = ERR_MEMORY;
        break;
      }
    } else {
      info->rejected++;
    }
  }

  return error;
}

