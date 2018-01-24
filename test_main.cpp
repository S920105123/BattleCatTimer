#include "logger.h"
#include "global.h"

int main()
{
    // Logger logger;
    logger.log(NORMAL) << "hello" ;
    logger.log() << "hello" ;
	logger.log(CERR)    << "A message from the universe is printed." << endl;
	logger.log(NORMAL)  << "This is a log example." << endl;
	logger.log(ERROR)   << "This is an error." << endl;
	logger.log(WARNING) << "This is a warning." << endl;
	logger.log()        << "Is it normal?" << endl;

	logger.turn_off();
	logger.log(ERROR) << "(This should not be printed) Life is like you just want to log something trivial but it still fails..." << endl;
	logger.turn_on();
	logger.log(ERROR) << "(After turn on) Hello everyone~" << endl;
    return 0;
}
