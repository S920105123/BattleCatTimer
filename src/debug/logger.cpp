#include "logger.h"

Logger* Logger::logger_instance = NULL;
int Logger::error_num = 0;
int Logger::warning_num = 0;

Logger* Logger::create() {
	if(logger_instance == NULL){
		logger_instance = new Logger();

		const std::string fname = "message.log";
		logger_instance->flog.open(fname);
		if (!logger_instance->flog.is_open()) {
			std::cerr << "[Error] - Cannot open log file, logger turned off." << endl;
		}
		logger_instance->on = logger_instance->flog.is_open();
		logger_instance->cur_stream = &logger_instance->flog;
		logger_instance->error_num = 0;
		logger_instance->warning_num = 0;
	}
	return logger_instance;
}

Logger::~Logger() {
	time_t now = time(NULL);
	char *tstr = ctime(&now);
	flog << "\n\n[Log] - "<<tstr<<"- ";
	flog << "close log files\n";
	std::cerr << "Log error : " << error_num << ", warning : " << warning_num << endl;
	flog.close();
}

Logger& Logger::Log(Log_type type, bool prefix) {

	if (logger_instance == NULL) logger_instance = create();

	if (!logger_instance->on) return *logger_instance;

	if (type != CERR) {
		logger_instance->cur_stream = &logger_instance->flog;
		if (prefix) {
			time_t now = time(NULL);
			char *tstr = ctime(&now);
			*(logger_instance->cur_stream) << "\n\n";
			if (type == NORMAL)       logger_instance->flog << "[Log] ";
			else if (type == WARNING) logger_instance->flog << "[Warning] ", logger_instance->warning_num++;
			else                      logger_instance->flog << "[Error] ", logger_instance->error_num++;
			*(logger_instance->cur_stream) << tstr;
		}
		*(logger_instance->cur_stream) << "- ";
	} else {
		logger_instance->cur_stream = &std::cerr;
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

// ----------------- For testing -----------------
#ifdef TEST_LOGGER

int main() {
	LOG(CERR)    << "A message from the universe is printed." << endl;
	LOG(NORMAL)  << "This is a log example." << endl;
	LOG(ERROR)   << "This is an error." << endl;
	LOG(WARNING) << "This is a warning." << endl;
	LOG()        << "Is it normal?" << endl;

	Logger::turn_off();
	LOG(ERROR) << "(This should not be printed) Life is like you just want to log something trivial but it still fails..." << endl;

	Logger::turn_on();
	LOG(ERROR) << "(After turn on) Hello everyone~" << endl;
	Logger::create()->~Logger();
}

#endif
