#ifndef DEBUG_H
#define DEBUG_H

#include "header.h"
#include "logger.h"

const bool ASSERT_FAILURE_CRASH = true; // Set false to disable crash from fail assertion
const bool NOT_REACHED = false;

#define DEBUG_MODE_ON // Comment this line to completely turn off debug mode.

#ifdef DEBUG_MODE_ON
	#define ASSERT(EXPR) assert_handler( (EXPR), #EXPR, __FILE__, __FUNCTION__, __LINE__)
	#define ASSERT_NOT_REACHED() ASSERT(NOT_REACHED)
	#define UNEXPECTED(NAME, UNEXPECT, EXPECT) \
		{LOG(CERR) << "\n[Fatal] Expectation fail, expect \"" << NAME << "\" to be \"" << (EXPECT) << "\", but it is \"" << (UNEXPECT) << "\"" <<  endl; \
		Logger::Log(CERR,false) << "- At \"" << __FILE__ << "\", \"" << __FUNCTION__ << "\", line " << __LINE__ << "\n\n";                                \
		LOG(ERROR) << "Expectation fail, expect variable \"" << NAME << "\" to be \"" << (EXPECT) << "\", but it is \"" << (UNEXPECT) << "\"" <<  endl;          \
		Logger::Log(ERROR,false) << "At \"" << __FILE__ << "\", \"" << __FUNCTION__ << "\", line " << __LINE__ << endl;}
	#define UNEXPECTED_NOT(NAME, UNEXPECT, EXPECT) \
		{LOG(CERR) << "\n[Fatal] Expectation fail, expect \"" << NAME << "\" not to be \"" << (EXPECT) << "\", but it is." << endl; \
		Logger::Log(CERR,false) << "- At \"" << __FILE__ << "\", \"" << __FUNCTION__ << "\", line " << __LINE__ << "\n\n";           \
		LOG(ERROR) << "Expectation fail, expect variable \"" << NAME << "\" not to be \"" << (EXPECT) << "\", but it is." << endl; \
		Logger::Log(ERROR,false) << "At \"" << __FILE__ << "\", \"" << __FUNCTION__ << "\", line " << __LINE__ << endl;}
	#define EXPECT(AGENT, TO_BE)              \
		if ((AGENT) != (TO_BE)) {             \
			UNEXPECTED(#AGENT, AGENT, TO_BE); \
			my_exit();             \
		}
	#define EXPECT_NOT(AGENT, TO_BE)              \
		if ((AGENT) == (TO_BE)) {             \
			UNEXPECTED_NOT(#AGENT, AGENT, TO_BE); \
			my_exit();             \
		}

#else
	#define ASSERT(EXPR) (EXPR);
	#define ASSERT_NOT_REACHED() ;
	#define UNEXPECTED(NAME, UNEXPECT, EXPECT) (NAME), (UNEXPECT), (EXPECT);
	#define UNEXPECTED_NOT(NAME, UNEXPECT, EXPECT) (NAME), (UNEXPECT), (EXPECT);
	#define EXPECT(AGENT, TO_BE) (AGENT), (TO_BE);
	#define EXPECT_NOT(AGENT, TO_BE) (AGENT), (TO_BE); 
#endif

void assert_handler(int expr, const char *assertion, const char *fname, const char *func, int line);
void my_exit();

#endif

