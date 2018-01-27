#include "debug.h"

void assert_handler(int expr, const char *fname, const char *func, int line) {
	if (!(expr)) {
        LOG(ERROR) << "Assertion fail at function \"" << func << "\"\n"
                   << "- At file \"" << fname << "\", line number "<< line << endl;
        LOG(CERR) << "\n\n[Fatal] Assertion fail at function \"" << func << "\"\n"
                  << "- At file \"" << fname << "\", line number "<< line << "\n\n\n";
		if (ASSERT_FAILURE_CRASH) {
			Logger::create()->~Logger();
			exit(EXIT_FAILURE);
		}
	}
}

// ----------------- For testing -----------------
#ifdef TEST_DEBUG

int main() {
	LOG(ERROR) << "NO~~~~" << endl;
	LOG(CERR)  << "You shall not pass" << endl;
	
	EXPECT(1,1);
	string reply = "Your calculus is failed.";
	EXPECT_NOT(reply, "You got an A+ in calculus.");
	EXPECT(reply, "You got an A+ in calculus.");
	ASSERT_NOT_REACHED();
}

#endif

