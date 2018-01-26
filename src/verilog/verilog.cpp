#include "verilog.h"

void Verilog::parse(const string &fname) {
	File_Reader reader;
	reader.open(fname);
	
	string tok;
	
	/* Read in the module name */
	ASSERT(reader.next_token() == "module");
	module_name = reader.next_token();
	
	/* Skip those parameters. */
	tok = reader.next_token();
	ASSERT(tok == "(");
	while (tok != ")") {
		tok = reader.next_token();
		if (tok.empty()) {
			ASSERT_NOT_REACHED();
			break;
		}
	}
	
	/* Reading inputs */
	tok = reader.next_token();
	while (tok == "input") {
		tok = reader.next_token();
		input.push_back(tok);
		tok = reader.next_token();
	}
	
	/* Reading outputs */
	while (tok == "output") {
		tok = reader.next_token();
		output.push_back(tok);
		tok = reader.next_token();
	}
	
	/* Reading wires */
	while (tok == "wire") {
		tok = reader.next_token();
		wire.push_back(tok);
		tok = reader.next_token();
	}
	
	/* Reading cells */
	while (tok != "endmodule") {
		if (tok.empty()) {
			ASSERT_NOT_REACHED();
			break;
		}
		
		gate.push_back(Verilog_Gate());
		Verilog_Gate &gt = gate.back();
		gt.cell_type = tok;
		gt.cell_name = reader.next_token();
		ASSERT(reader.next_token() == "(");
		tok = reader.next_token();  // In case empty argument list.
		while (tok != ")") {
			if (tok.empty()) {
				ASSERT_NOT_REACHED();
				break;
			}
			string inp, from;
			ASSERT(tok == ".");
			inp  = reader.next_token();
			ASSERT(reader.next_token() == "(");
			from = reader.next_token();
			ASSERT(reader.next_token() == ")");
			gt.param.push_back({inp, from});
			tok = reader.next_token();
		}
		tok = reader.next_token();
	}
	
	LOG(NORMAL) << "Parse of verilog file is done." << endl;
}

// --------------- For Test ---------------

#ifdef TEST_VERILOG

int main() {
	Verilog vlog;
	vlog.parse("unit_test/verilog_test.v");
	
	LOG(CERR) << "Parse done\n\n";
	LOG(CERR) << "Module name: " << vlog.module_name << "\n\n";
	
	LOG(CERR) << "Input list:" << endl;
	for (const string &inp : vlog.input) {
		LOG(CERR) << "    " << inp << endl;
	}
	LOG(CERR) << endl;
	
	LOG(CERR) << "Output list:" << endl;
	for (const string &out : vlog.output) {
		LOG(CERR) << "    " << out << endl;;
	}
	LOG(CERR) << endl;
	
	LOG(CERR) << "Wire list:" << endl;
	for (const string &w : vlog.wire) {
		LOG(CERR) << "    " << w << endl;;
	}
	LOG(CERR) << endl;
	
	LOG(CERR) << "Cells: " << endl;
	for (const Verilog::Verilog_Gate &gt : vlog.gate) {
		LOG(CERR) << "    - Type: "<< gt.cell_type << endl;
		LOG(CERR) << "    - Name: "<< gt.cell_name << endl;
		LOG(CERR) << "    - Parameters:" << endl;
		for (const pair<string,string> &p : gt.param) {
			LOG(CERR) << "          ." << p.first << "(" << p.second << ")" << endl;
		}
		LOG(CERR) << endl;
	}
}

#endif
