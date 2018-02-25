#include "verilog.h"

void Verilog::open(const string &fname) {
	File_Reader reader;
	reader.open(fname);

	string tok;

	/* Read in the module name */
	EXPECT(reader.next_token(), "module");
	module_name = reader.next_token();

	/* Skip those parameters. */
	tok = reader.next_token();
	EXPECT(tok, "(");
	while (tok != ")") {
		tok = reader.next_token();
		EXPECT_NOT(tok, "");
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
		EXPECT_NOT(tok, "");
		Verilog_Gate *gt = new Verilog_Gate;
		gt->cell_type = tok;
		gt->cell_name = reader.next_token();
		EXPECT(reader.next_token(), "(");
		tok = reader.next_token();  // In case empty argument list.
		while (tok != ")") {
			EXPECT_NOT(tok, "");
			string inp, from;
			EXPECT(tok, ".");
			inp  = reader.next_token();
			EXPECT(reader.next_token(), "(");
			from = reader.next_token();
			EXPECT(reader.next_token(), ")");
			gt->param.push_back({inp, from});
			tok = reader.next_token();
		}
		tok = reader.next_token();
		gates.emplace(gt->cell_name, gt);
	}

	LOG(NORMAL) << "Parse of verilog file is done." << '\n';
}

const string& Verilog::get_cell_type(const string &inst_name) const {
	static const string undefined_str = "Undefined";
	auto it = gates.find(inst_name);
	if (it == gates.end()) {
		LOG(WARNING) << "[Verilog] Searching for an unknown verilog gate instance name" << '\n';
		return undefined_str;
	}
	return it->second->cell_type;
}

Verilog::~Verilog() {
	for (auto &it_pair : this->gates) {
		delete it_pair.second;
	}
}

// --------------- For Test ---------------

#ifdef TEST_VERILOG

int main() {
	Verilog vlog;
	vlog.parse("unit_test/test_verilog.v");

	LOG(CERR) << "Parse done\n\n";
	LOG(CERR) << "Module name: " << vlog.module_name << "\n\n";

	LOG(CERR) << "Input list:" << '\n';
	for (const string &inp : vlog.input) {
		LOG(CERR) << "    " << inp << '\n';
	}
	LOG(CERR) << '\n';

	LOG(CERR) << "Output list:" << '\n';
	for (const string &out : vlog.output) {
		LOG(CERR) << "    " << out << '\n';;
	}
	LOG(CERR) << '\n';

	LOG(CERR) << "Wire list:" << '\n';
	for (const string &w : vlog.wire) {
		LOG(CERR) << "    " << w << '\n';;
	}
	LOG(CERR) << '\n';

	LOG(CERR) << "Cells: " << '\n';
	for (auto it : vlog.gates) {
		const Verilog::Verilog_Gate &gt = *it.second;
		LOG(CERR) << "    - Type: "<< gt.cell_type << '\n';
		LOG(CERR) << "    - Name: "<< gt.cell_name << '\n';
		LOG(CERR) << "    - Parameters:" << '\n';
		for (const pair<string,string> &p : gt.param) {
			LOG(CERR) << "          ." << p.first << "(" << p.second << ")" << '\n';
		}
		LOG(CERR) << '\n';
	}

	string to_find = "U7";
	LOG(CERR) << "Find instance " << to_find << ". Result is: " << vlog.get_cell_type(to_find) << '\n';
}

#endif
