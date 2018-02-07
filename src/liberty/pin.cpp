// #include "pin.h"
#include "liberty.h"

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
			TimingArc *arc = new TimingArc(cell_lib);
			arc->read(in);
			arc->set_cell_ptr(parent);
			arc->set_to_pin(name);
			ASSERT(cell_lib!=NULL);
			if(cell_lib->get_timing_mode()==Mode::EARLY){
				if( arc->get_timing_type() != Timing_Type::SETUP_RISING and
					arc->get_timing_type() != Timing_Type::SETUP_FALLING)
					this->add_arc( arc->get_related_pin(), arc);
			}else{ // Late
				if( arc->get_timing_type() != Timing_Type::HOLD_RISING and
					arc->get_timing_type() != Timing_Type::HOLD_FALLING)
					this->add_arc( arc->get_related_pin(), arc);
			}
			// this->timing.insert( make_pair(arc->get_related_pin(), arc) );
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

void Pin::add_arc(const string &src, TimingArc* arc){
	if(timing.find(src) == timing.end()){
		timing[src] = new vector<TimingArc*>;
	}
	timing[src]->emplace_back( arc );
	total_timingArc->emplace_back(arc);
}

void Pin::print(const string &tab) {
	string next_level = tab+"    ";
	LOG(CERR) << tab << "Pin name: " << this->name << endl;
	LOG(CERR) << tab << "	- Direction: " << this->direction << endl;
	LOG(CERR) << tab << "	- Is Clock: " << (this->is_clock ? "True" : "False") << endl;
	LOG(CERR) << tab << "	- Capacitance: " << this->capacitance << endl;
	LOG(CERR) << tab << "	- Max capacitance: " << this->max_capacitance << endl;
	LOG(CERR) << tab << "	- Min capacitance: " << this->min_capacitance << endl;
	LOG(CERR) << tab << "	- Timing Arcs: " << this->min_capacitance << endl;
	for (const auto &it : timing) {
		for(const auto &arc: *it.second)
			arc->print(next_level);
	}
	LOG(CERR) << endl;
}

float Pin::get_capacitance(){
	return capacitance;
}

bool Pin::get_is_clock(){
	return is_clock;
}

vector<TimingArc*>* Pin::get_TimingArc(const string& src){
	auto it = timing.find(src);
	if(it == timing.end()) {
		LOG(ERROR) << "[Pin][get_TimingArc] Cell: " << parent->get_type_name()
		<< " pin: " << name << " src: " << src << " empty TimingArc." << endl;
		return NULL;
	}
	return it->second;
}

vector<TimingArc*>* Pin::get_total_TimingArc(){
	return total_timingArc;
}

Direction_type Pin::get_direction(){
	return tolower(direction[0])=='o' ? OUTPUT : INPUT;
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
