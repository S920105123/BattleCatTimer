#include "logger.h"

Logger* Logger::logger_instance = NULL;
int Logger::error_num = 0;
int Logger::warning_num = 0;
vector<pair<string,int>> Logger::timestamp;
map<string, int> Logger::record;
map<string, long long> Logger::time_record;
long long Logger::start_time;

Logger* Logger::create() {
	if(logger_instance == NULL){
		logger_instance = new Logger();

		const std::string fname = "message.log";
		logger_instance->flog.open(fname);
		if (!logger_instance->flog.is_open()) {
			std::cerr << "[Error] - Cannot open log file, logger turned off." << '\n';
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
	int pre = 0, total = 0;

	int space = 0;
	for(const auto& p: timestamp) {
		space = std::max(space, (int)p.first.size());
	}

	for(const auto& re: record) {
		std::cerr << re.first << " : " << re.second << '\n';
	}

	for (const auto &p : timestamp) {
		if(pre==0) pre = p.second;
		total += p.second-pre;
		std::cerr << std::setw(space+1) << p.first << "| delay: " << std::setw(3) << p.second - pre << "| total: " << std::setw(3) << total << '\n';
		pre = p.second;
	}
	std::cerr << "Log error : " << error_num << ", warning : " << warning_num << '\n';

	for(const auto &p: time_record) {
		std::cerr << p.first << " : " << p.second << " ms\n";
	}
	flog.close();
}

void Logger::add_timestamp(const string& event){
	timestamp.emplace_back(event, time(NULL));

	 std::cerr << event << '\n' << std::flush;
}

void Logger::add_record(const string& name, int val) {
	record[ name ] += val;
}

void Logger::start() {
    start_time = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()
	).count();
}

void Logger::stop(const string& x){ // add time elapsed to record
	long long now = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()
	).count();
	time_record[x] += now - start_time;
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
	LOG(CERR)    << "A message from the universe is printed." << '\n';
	LOG(NORMAL)  << "This is a log example." << '\n';
	LOG(ERROR)   << "This is an error." << '\n';
	LOG(WARNING) << "This is a warning." << '\n';
	LOG()        << "Is it normal?" << '\n';

	Logger::turn_off();
	LOG(ERROR) << "(This should not be printed) Life is like you just want to log something trivial but it still fails..." << '\n';

	Logger::turn_on();
	LOG(ERROR) << "(After turn on) Hello everyone~" << '\n';
	Logger::create()->~Logger();
}

#endif
