#ifndef VERILOG_H
#define VERILOG_H

#include "header.h"
#include "debug.h"
#include "file_reader.h"

struct Verilog {
	struct Verilog_Gate {
		// For example
		// NOR_GATE gt( .a(net1), .b(net2), .o(net3) );
		// Will be stored as
		// cell_type: "NOR_GATE"
		// cell_name: "gt"
		// param: {
		//     ("a", "net1"),
		//     ("b", "net2"),
		//     ("o", "net3"),
		// }
		string cell_type, cell_name;
		vector< pair<string,string> > param;
	};

	~Verilog();
	void open(const string &fname); // Use this function to initiate from a .v file.
	const string& get_cell_type(const string &inst_name) const;

	string module_name;
	vector<string> wire;             // Wire objects
	vector<string> input, output;    // Primary input, output
	unordered_map<string, Verilog_Gate*> gates;
};

#endif
