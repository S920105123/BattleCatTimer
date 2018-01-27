#include "cell_lib.h"

int main()
{
	string filename;
	CellLib lib;
	cout << "enter filename : ";
	cin  >> filename;
    lib.open(filename);
	cout << "open " << filename << "ok\n";

	cout << "total cells: " << lib.cells_size() << endl;
	cout << "template table : ";
	lib.print_template();

	string name;
    while(true){
		cout << "enter cell type or exit : ";
		cin >> name;
		if(name=="exit") break;
		lib.print_cell(name);
    }
	Logger* logger = Logger::create();
    logger->~Logger();
    return 0;
}
