#ifndef LOGGER_H
#define LOGGER_H

#include "header.h"
#define endl '\n'     // So that logger can use it.
#define LOG(TYPE) Logger::Log(TYPE)
#define LOG_PLACE(TYPE) Logger::Log(TYPE, flase) << "In " << __FILE__ << ", line number: " << __LINE__ << endl;

typedef enum { NORMAL, WARNING, ERROR, CERR } Log_type;

class Logger {

public:
	~Logger();                 // Including close log files.
	static Logger* create();   // Creat Logger by this function
	static void turn_off();    // Turn off logger (Logger is on by default)
	static void turn_on();     // Turn on logger (Logger is on by default)

	// Type may be NORMAL, WARNING, ERROR or CERR, print date if "prefix" is true.
	static Logger& Log(Log_type type = NORMAL, bool prefix = true);

	template <class T>
	Logger& operator<<(const T &to_log) {
		if (on) {
			(*cur_stream) << to_log << std::flush;
		}
		return *this;
	}
private:
	Logger(){};                          // can't be created in public
	Logger(Logger const&){};		     // can't be copied in public

	static Logger* logger_instance;
	static int error_num, warning_num;
	bool on;
	std::ofstream flog;
	std::ostream *cur_stream;

};

#endif
