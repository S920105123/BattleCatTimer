#include "logger.h"
#include "file_reader.h"

int main()
{
	Logger* logger = Logger::create();
    // Logger logger;
    LOG(NORMAL) << "hello" ;
    LOG() << "hello" ;
	LOG(CERR)   << "A message from the universe is printed." << endl;
	LOG(NORMAL)  << "This is a log example." << endl;
	LOG(ERROR)   << "This is an error." << endl;
	LOG(WARNING) << "This is a warning." << endl;
	LOG()        << "Is it normal?" << endl;

	Logger::turn_off();
	LOG(ERROR) << "(This should not be printed) Life is like you just want to log something trivial but it still fails..." << endl;
	Logger::turn_on();
	LOG(ERROR) << "(After turn on) Hello everyone~" << endl;

    File_Reader reader;
    reader.open("testcase_v1.2\\s1196\\s1196_Early.lib");
    // reader.open("testcase_v1.2\\s1196\\s1196.v");
    // reader.open("src\\parser\\file_reader\\testcase.test");
    // reader.open("testcase_v1.2\\s1196\\s1196.spef");
    freopen("test_out.test","w",stdout);
    while(true){
        string token = reader.next_token();
        if(token.empty()) break;
        cout << token << endl;
    }
    logger->~Logger();
    return 0;
}
