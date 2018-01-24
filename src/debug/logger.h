#ifndef LOGGER_H
#define LOGGER_H

#include "header.h"
#define endl '\n'     // So that logger can use it.

typedef enum { NORMAL, WARNING, ERROR, CERR } Log_type;

class Logger {
	
public:
	Logger();                             // Initialize the logger, including open log files.
	~Logger();                            // Including close log files.
	inline void turn_off();               // Turn off logger (Logger is on by default)
	inline void turn_on();                // Turn on logger (Logger is on by default)
	inline Logger& log(Log_type type);    // Type may be NORMAL, WARNING, ERROR or CERR
	template <class T> Logger& operator<<(const T &to_log);

private:
	bool on;
	std::ofstream ferr, fwarn, fnorm;
	std::ostream *cur_stream;

};

#endif

