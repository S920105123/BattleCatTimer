#include "cell_lib.h"

int main()
{
	CellLib lib;
    lib.open("testcase_v1.2\\s1196\\s1196_Early.lib");
	// spef.open("testcase.test");
    // freopen("test_out.test","w",stdout);
	cout << "open ok\n";
	cout << "total cells: " << lib.cells_size() << enld;
	cout << "template table : ";
	libs.print_template();

	string name;
    while(cin >> name){
		if(name=="exit") break;
		libs.print_cell(name);
    }
	Logger* logger = Logger::create();
    logger->~Logger();
    return 0;
}
