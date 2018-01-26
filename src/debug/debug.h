#ifndef DEBUG_H
#define DEBUG_H

#include "header.h"
#include "logger.h"

const bool DEBUG_ON = true; // Set false to disable crash from fail assertion

#define ASSERT(EXPR) assert_handler((EXPR), __FILE__, __FUNCTION__, __LINE__)
#define ASSERT_NOT_REACHED() ASSERT(false)

void assert_handler(int expr, const char *fname, const char *func, int line);

#endif

