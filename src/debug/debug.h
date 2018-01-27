#ifndef DEBUG_H
#define DEBUG_H

#include "header.h"
#include "logger.h"

const bool DEBUG_ON = true; // Set false to disable crash from fail assertion

#define ASSERT(EXPR) assert_handler((EXPR), __FILE__, __FUNCTION__, __LINE__)

#define ASSERT_NOT_REACHED() ASSERT(false)

#define UNEXPECTED(UNEXPECT, EXPECT) \
	LOG(CERR) << "\n[Error] Unexpected file format, expect \"" << (EXPECT) << "\", but get \"" << (UNEXPECT) << "\"" <<  endl; \
	Logger::Log(CERR,false) << "- At \"" << __FILE__ << "\", \"" << __FUNCTION__ << "\", line " << __LINE__ << endl;           \
	LOG(ERROR) << "Unexpected format, expect \"" << (EXPECT) << "\", but get \"" << (UNEXPECT) << "\"" << endl;                \
	Logger::Log(ERROR,false) << "At \"" << __FILE__ << "\", \"" << __FUNCTION__ << "\", line " << __LINE__ << endl;

#define EXPECT(AGENT, TO_BE)     \
	if ((AGENT) != (TO_BE)) {      \
		UNEXPECTED(AGENT,TO_BE); \
		ASSERT_NOT_REACHED();    \
	}

void assert_handler(int expr, const char *fname, const char *func, int line);

#endif

