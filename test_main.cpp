#include "logger.h"
#include "spef.h"
#include "file_reader.h"

int main()
{
	Spef spef;
    spef.open("testcase_v1.2\\s1196\\s1196.spef");
	// spef.open("testcase.test");
    // freopen("test_out.test","w",stdout);
	cout << "open ok\n";
	string name;
    while(cin >> name){
		if(name=="exit") break;
		spef.print_net(name);
    }
	Logger* logger = Logger::create();
    logger->~Logger();
    return 0;
}
