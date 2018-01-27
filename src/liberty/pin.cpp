#include "pin.h"

void Pin::read(File_Reader &in){
	int level = 1;   // Parenthesis level
	string tok;
	this->is_clock = false;
	
	EXPECT(in.next_token(), "{");
	tok = in.next_token();
	while (tok != "}" || level!=1) {
		EXPECT_NOT(tok, "");
		if (tok == "direction") {
			EXPECT(in.next_token(), ":");
			this->direction = in.next_token();
		}
		else if (tok == "clock" ) {
			EXPECT(in.next_token(), ":");
			if (in.next_token() == "true") {
				this->is_clock = true;
			}
		}
		else if (tok == "capacitance") {
			EXPECT(in.next_token(), ":");
			this->capacitance = std::stof(in.next_token());
		}
		else if (tok == "max_capacitance") {
			EXPECT(in.next_token(), ":");
			this->max_capacitance = std::stof(in.next_token());
		}
		else if (tok == "min_capacitance") {
			EXPECT(in.next_token(), ":");
			this->min_capacitance = std::stof(in.next_token());
		}
		else if (tok == "timing") {
			EXPECT(in.next_token(), "(");
			EXPECT(in.next_token(), ")");
//			TimingArc *arc = new TimingArc;
//			arc->read(in);
//			this->timing.insert( make_pair(arc->get_related_pin(), arc) );
		}
		else {
			if (tok == "}") level--;
			if (tok == "{") level++;
			LOG(WARNING) << "An unknown keyword \"" << tok << "\" is reached. (In .lib pin parser)" <<endl;
		}
		tok = in.next_token();
	}
}

void Pin::set_name(const string &new_name) {
	this->name = new_name;
}

void Pin::set_parent(Cell *p) {
	this->parent = p;
}

void Pin::print(const string &tab) {
	string next_level = tab+"    ";
	LOG(CERR) << tab << "Pin name: " << this->name << endl;
	LOG(CERR) << tab << "Is Clock: " << (this->is_clock ? "True" : "False") << endl;
	LOG(CERR) << tab << "Capacitance: " << this->capacitance << endl;
	LOG(CERR) << tab << "Max capacitance: " << this->max_capacitance << endl;
	LOG(CERR) << tab << "Min capacitance: " << this->min_capacitance << endl;
	LOG(CERR) << tab << "Timing Arcs: " << endl;
//	for (const auto &it : timing) {
//		it.second->print(next_level);
//	}
}

// ----------- For testing --------------

#ifdef TEST_PIN

int main() {
	Pin p;
	File_Reader freader;
	freader.open("unit_test/test_pin.lib");
	p.read(freader);
	p.print();
	Logger::create()->~Logger();
}

#endif
