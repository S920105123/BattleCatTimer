#include "logger.h"

Logger* Logger::logger_instance = NULL;

Logger* Logger::create() {
	if(logger_instance == NULL){
		logger_instance = new Logger();

		const std::string fname = "message.log";
		logger_instance->on = true;
		logger_instance->flog.open(fname);
		logger_instance->cur_stream = &logger_instance->flog;
	}
	return logger_instance;
}

Logger::~Logger() {
	flog << "[Log] close log files\n";
	flog.close();
}

Logger& Logger::Log(Log_type type) {

	if (logger_instance == NULL) logger_instance = create();

	if (!logger_instance->on) return *logger_instance;

	if (type != CERR) {
		logger_instance->cur_stream = &logger_instance->flog;
		time_t now = time(NULL);
		char *tstr = ctime(&now);
		*(logger_instance->cur_stream) << "\n\n"<<tstr<<"- ";
		if (type == NORMAL)       logger_instance->flog << "[Log] ";
		else if (type == WARNING) logger_instance->flog << "[Warning] ";
		else                      logger_instance->flog << "[Error] ";
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
	 logger->~Logger();
}

#endif
