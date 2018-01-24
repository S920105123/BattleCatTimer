#include "logger.h"

Logger* Logger::logger_instance = NULL;

Logger* Logger::create() {
	if(logger_instance == NULL){
		logger_instance = new Logger();

		const std::string fnames[] = {
			"log_normal.log",
			"log_warning.log",
			"log_error.log"
		};
		logger_instance->on = true;
		logger_instance->fnorm.open(fnames[0]);
		logger_instance->fwarn.open(fnames[1]);
		logger_instance->ferr.open(fnames[2]);
		logger_instance->cur_stream = &logger_instance->fnorm;
	}
	return logger_instance;
}

Logger::~Logger() {
	fnorm << "[Log] close log files\n";
	fnorm.close();
	fwarn.close();
	ferr.close();
}

Logger& Logger::Log(Log_type type) {

	if(logger_instance == NULL) logger_instance = create();

	if (type == NORMAL)       logger_instance->cur_stream = &logger_instance->fnorm;
	else if (type == WARNING) logger_instance->cur_stream = &logger_instance->fwarn;
	else if (type == ERROR)   logger_instance->cur_stream = &logger_instance->ferr;
	else                      logger_instance->cur_stream = &std::cerr;

	if (type != CERR && logger_instance->on) {
		time_t now = time(NULL);
		char *tstr = ctime(&now);
		*(logger_instance->cur_stream) << "\n\n"<<tstr<<"- ";
	}

	return *logger_instance;
}

void Logger::turn_off() {
	if(logger_instance)
		logger_instance->on = false;
}

void Logger::turn_on() {
	if(logger_instance)
		logger_instance->on = true;
}

// template <class T> Logger& Logger::operator<<(const T &to_log) {
// 	if (on) {
// 		(*cur_stream) << to_log;
// 	}
// 	return *this;
// }


// ----------------- For testing -----------------
#ifdef TEST_LOGGER

int main() {
	Logger* logger = Logger::create();
	LOG(CERR)    << "A message from the universe is printed." << endl;
	LOG(NORMAL)  << "This is a log example." << endl;
	LOG(ERROR)   << "This is an error." << endl;
	LOG(WARNING) << "This is a warning." << endl;
	LOG()        << "Is it normal?" << endl;

	Logger::turn_off();
	LOG(ERROR) << "(This should not be printed) Life is like you just want to log something trivial but it still fails..." << endl;

	Logger::turn_on();
	LOG(ERROR) << "(After turn on) Hello everyone~" << endl;
	// logger->~Logger();
}

#endif
