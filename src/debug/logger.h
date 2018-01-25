#ifndef LOGGER_H
#define LOGGER_H

#include "header.h"
#define endl '\n'     // So that logger can use it.
#define LOG(TYPE) Logger::Log(TYPE)

typedef enum { NORMAL, WARNING, ERROR, CERR } Log_type;

class Logger {

public:
	~Logger();                      		 	 // Including close log files.
	static Logger* create();        		 	 // Creat Logger by this function
	static void turn_off();          	 	     // Turn off logger (Logger is on by default)
	static void turn_on();               	   	 // Turn on logger (Logger is on by default)
	static Logger& Log(Log_type type = NORMAL);  // Type may be NORMAL, WARNING, ERROR or CERR

	template <class T>
	Logger& operator<<(const T &to_log) {
		if (on) {
			(*cur_stream) << to_log;
		}
		return *this;
	}
private:
	Logger(){};                          // can't be created in public
	Logger(Logger const&){};		     // can't be copied in public

	static Logger* logger_instance;
	bool on;
	std::ofstream flog;
	std::ostream *cur_stream;

};

#endif
