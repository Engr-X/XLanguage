#ifndef _LIBS_FILE_HELPER_H_
#define _LIBS_FILE_HELPER_H_

#include <stdio.h>
#include <string.h>

#include "lib/core_types.h"

bool file_read(const char* path, const char* encoding, char* dst);
bool file_write(const char* path, const char* content, bool append);
bool file_delete(const char* path);

#endif