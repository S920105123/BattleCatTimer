#include "logger.h"

Logger::Logger() {
	const std::string fnames[] = {
		"normal.log",
		"warning.log",
		"error.log"
	};
	fnorm.open(fnames[0]);
	fwarn.open(fnames[1]);
	ferr.open(fnames[2]);
	cur_stream = &fnorm;
}

Logger::~Logger() {
	fnorm.close();
	fwarn.close();
	ferr.close();
}

inline Logger& Logger::log(Log_type type = NORMAL) {
	
	if (type == NORMAL)       cur_stream = &fnorm;
	else if (type == WARNING) cur_stream = &fwarn;
	else if (type == ERROR)   cur_stream = &ferr;
	else                      cur_stream = &std::cerr;
	if (type != CERR && on) {
		time_t now = time(NULL);
		char *tstr = ctime(&now);
		(*cur_stream) << "\n\n"<<tstr<<"- ";
	}
	
	return *this;
}

inline void Logger::turn_off() {
	on = false;
}

inline void Logger::turn_on() {
	on = true;
}

template <class T> Logger& Logger::operator<<(const T &to_log) {
	if (on) {
		(*cur_stream) << to_log;
	}
	return *this;
}


// ----------------- For testing -----------------
#ifdef TEST_LOGGER

int main() {
	Logger logger;
	logger.log(CERR)    << "A message from the universe is printed." << endl;
	logger.log(NORMAL)  << "This is a log example." << endl;
	logger.log(ERROR)   << "This is an error." << endl;
	logger.log(WARNING) << "This is a warning." << endl;
	logger.log()        << "Is it normal?" << endl; 
	
	logger.turn_off();
	logger.log(ERROR) << "(This should not be printed) Life is like you just want to log something trivial but it still fails..." << endl;
	logger.turn_on();
	logger.log(ERROR) << "(After turn on) Hello everyone~" << endl;
}

#endif
