#ifndef LOGGER_H
#define LOGGER_H

#include "header.h"

#define LOG(TYPE) Logger::Log(TYPE)
#define LOG_PLACE(TYPE) Logger::Log(TYPE, flase) << "In " << __FILE__ << ", line number: " << __LINE__ << '\n';

typedef enum { NORMAL, WARNING, ERROR, CERR } Log_type;

class Logger {

public:
	~Logger();                 // Including close log files.
	static Logger* create();   // Creat Logger by this function
	static void turn_off();    // Turn off logger (Logger is on by default)
	static void turn_on();     // Turn on logger (Logger is on by default)
	static void add_timestamp(const string& event);
	static void add_record(const string& name, int val);
	static void start();
	static void stop(const string& ); // add time elapsed to record

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
	static vector<pair<string,int>> timestamp;
	static map<string, int> record;
	static map<string, long long> time_record;
	static long long start_time;
	bool on;
	std::ofstream flog;
	std::ostream *cur_stream;

};

#endif
