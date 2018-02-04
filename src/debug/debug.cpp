#include "debug.h"

void assert_handler(int expr, const char *assertion, const char *fname, const char *func, int line) {
	if (!(expr)) {
        LOG(ERROR) << "Assertion \"" << assertion << "\" fail at function \"" << func << "\"\n"
                   << "- At file \"" << fname << "\", line number "<< line << endl;
        LOG(CERR) << "\n\n[Fatal] Assertion \"" << assertion << "\" fail at function \"" << func << "\"\n"
                  << "- At file \"" << fname << "\", line number "<< line << "\n\n\n";
		my_exit();
	}
}

void my_exit() {
	if (ASSERT_FAILURE_CRASH) {
		Logger::create()->~Logger();
		exit(EXIT_FAILURE);
	}
}

// ----------------- For testing -----------------
#ifdef TEST_DEBUG

int main() {
	LOG(ERROR) << "NO~~~~" << endl;
	LOG(CERR)  << "You shall not pass" << endl;
	
//	ASSERT_NOT_REACHED();
//	ASSERT(1==0);
	EXPECT(1,1);
	string reply = "Your calculus is failed.";
	EXPECT_NOT(reply, "You got an A+ in calculus.");
	EXPECT(reply, "You got an A+ in calculus.");
}

#endif

