#include "cell.h"

void Cell::read(File_Reader &in){
    int level = 1;
    LOG(NORMAL) << "[Cell][read] " << type_name << endl;
    EXPECT(in.next_token(), "{");
    string token, name;
    while(level){
        token = in.next_token();
        if(token=="}") level--;
        else if(token=="{") level++;
        else if(token=="pin"){
            EXPECT(in.next_token(), "(");
            name = in.next_token();
            EXPECT(in.next_token(), ")");

            Pin *pin = new Pin();
            pin->set_name(name);
            pin->set_parent(this);
            pin->read(in);
            add_pin(name, pin);
        }
        else{
            LOG(WARNING) << "[Cell][read] unknow keyword: " << token << endl;
        }
    }
}

void Cell::set_name(string name){
    type_name = name;
}

void Cell::add_pin(string name, Pin* pin){
    if(pins.find(name)==pins.end()){
        pins[name] = pin;
    }else LOG(ERROR) << "[Cell][add_pin] " << name << " appear twice.\n";
}

void Cell::print(const string &tab){
    LOG(CERR) << tab << " Cell : " << type_name << endl;

    string nextTab = tab + "    ";
    for(auto i:pins) i->print(nextTab);

    LOG(CERR) << endl;
}
