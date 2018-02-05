#include "liberty.h"
#include "verilog.h"
#include "spef.h"
#include "rc_tree.h"

Spef* spef;
Verilog* verilog;
CellLib* lib[2];

void cell_delay(){
	string cell_type, pin_name, related_pin;
	string from_s, to_s;
	float x, y;
	while(true){
		cout << "enter cell type or exit : ";
		cin >> cell_type;
		if(cell_type=="exit") break;
		cout << "enter pin_name(to): ";
		cin >> pin_name;

		cout << "enter related_pin(from): ";
		cin >> related_pin;

		int mode;
		cout << "enter Mode(0:Early, 1:Late): ";
		cin >> mode;

		Pin* pin = lib[mode]->get_pin_ptr(cell_type, pin_name);
		if(pin!=NULL) cout << "pin get!\n";
		else continue;

		vector<TimingArc*>* timingarc = pin->get_TimingArc(related_pin);
		if(timingarc!=NULL) cout << "timing arc get!\n";
		else continue;

		cout << "timing arc size = " << (*timingarc).size() << endl;
		for(auto it:*timingarc){
			cout << "timing arc: " << it->get_timing_sense_string() << " " << it->get_timing_type_string() << endl;
		}

		cout << "enter fom_value(input_slew) and to_value(c_load(delay,slew) or ouput_slew(constraint)): ";
		cin >> x >> y;

		cout << "enter from's Transition_Type(FALL or RISE): ";
		cin >> from_s;
		cout << "enter to's Transition_Type(FALL or RISE): ";
		cin >> to_s;

		Transition_Type from = from_s=="FALL"? FALL:RISE;
		Transition_Type to   = to_s=="FALL"? FALL:RISE;

		int cas;
		cout << "enter case: 1. delay, 2.slew, 3.constraint 4. All: ";
		cin >> cas;
		for(auto it:*timingarc){
			// it->print("  ");
			if(cas == 1 or cas==4){
				float delay = it->get_delay(from, to, x, y);
				cout << "\ndelay = " << delay << endl;
			}
			if(cas==2 or cas==4){
				float slew = it->get_slew(from, to, x, y);
				cout << "\nslew = " << slew << endl;
			}
			if(cas==3 or cas==4){
				float constraint = it->get_constraint(from, to, x, y);
				cout << "\nconstraint = " << constraint << endl;
			}
		}
	}
}

void rc_delay()
{
	string net_name;
	while(true){
		cout << "enter net_name or exit: ";
		cin >> net_name;
		if(net_name=="exit") break;

	    SpefNet* spefnet = spef->get_spefnet_ptr(net_name);
	    if(spefnet==NULL) continue;
	    spef->print_net(net_name);
	    RCTree rc(spefnet, verilog, lib);
		rc.build_tree();
		if(net_name=="out") rc.add_pin_cap("out", 4); // simple.timing load out 4
		rc.cal();

		string which, name;
		cout << "enter delay, slew or downstream or all: ";
		cin >> which;
		cout << "enter name: ";
		cin >> name;
		int mode;
		cout << "enter Mode(0:Early, 1:Late): ";
		cin >> mode;
		if(which=="delay" or which=="all"){
			cout << "delay = " << rc.get_delay(static_cast<Mode>(mode), name) << endl;
		}
		if(which=="downstream" or which=="all"){
			cout << "downstream = " << rc.get_downstream(static_cast<Mode>(mode), name) << endl;
		}
		if(which=="slew" or which=="all"){
			float input_slew;
			string tmp;
			while(true){
				cout << "enter input_slew or exit: ";
				cin >> tmp;
				if(tmp=="exit") break;
				input_slew = stof(tmp);
				cout << "input_slew = " << input_slew << ", slew = " << rc.get_slew(static_cast<Mode>(mode), name, input_slew) << endl;
			}
		}
	}
}

int main()
{
	string testcase;
	while(true){
		cout << "enter testcase(simple, s1196, ...) or exit: ";
		cin  >> testcase;
		if(testcase=="exit") break;

		spef = new Spef();
		verilog = new Verilog();
		lib[EARLY] = new CellLib(Mode::EARLY);
		lib[LATE] = new CellLib(Mode::LATE);

		string tmp = testcase + "/" + testcase;
		spef->open("testcase_v1.2/" + tmp  + ".spef");
		verilog->open("testcase_v1.2/" + tmp + ".v");
		lib[EARLY]->open("testcase_v1.2/" + tmp + "_Early.lib");
		lib[LATE]->open("testcase_v1.2/" + tmp + "_Late.lib");
		cout << "open " << testcase << " ok\n";

		cout << "Early: total cells: " << lib[EARLY]->cells_size() << endl;
		cout << "template table: ";
		lib[EARLY]->print_template();
		cout << endl;
		cout << "Late: total cells: " << lib[LATE]->cells_size() << endl;
		cout << "template table: ";
		lib[LATE]->print_template();

		string mod;
		while(true){
			cout << "enter mod(rc_delay or cell_delay) or exit: ";
			cin >> mod;
			if(mod=="exit") break;
			if(mod=="cell_delay") cell_delay();
			else rc_delay();
		}
	}
	Logger* logger = Logger::create();
    logger->~Logger();
    return 0;
}
