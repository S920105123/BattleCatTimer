#ifndef LOGGER_H
#define LOGGER_H

#include "header.h"
#define endl '\n'     // So that logger can use it.

typedef enum { NORMAL, WARNING, ERROR, CERR } Log_type;

class Logger {

public:
	Logger();                             // Initialize the logger, including open log files.
	~Logger();                            // Including close log files.
	void turn_off();               // Turn off logger (Logger is on by default)
	void turn_on();                // Turn on logger (Logger is on by default)
	Logger& log(Log_type type = NORMAL);    // Type may be NORMAL, WARNING, ERROR or CERR

	template <class T>
	Logger& operator<<(const T &to_log) {
		if (on) {
			(*cur_stream) << to_log;
		}
		return *this;
	}
private:
	bool on;
	std::ofstream ferr, fwarn, fnorm;
	std::ostream *cur_stream;

};

#endif
