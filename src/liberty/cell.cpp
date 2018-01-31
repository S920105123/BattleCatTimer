#include "cell.h"

void Cell::read(File_Reader &in){
    int level = 1;
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

            Pin *pin = new Pin(cell_lib);
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

void Cell::set_name(const string& name){
    type_name = name;
}

void Cell::add_pin(const string& name, Pin* pin){
    if(pins.find(name)==pins.end()){
        pins[name] = pin;
    }else LOG(ERROR) << "[Cell][add_pin] " << name << " appear twice.\n";
}

void Cell::print(const string &tab){
    LOG(CERR) << tab << " Cell : " << type_name << endl;

    string nextTab = tab + "    ";
    for(auto i:pins) i.second->print(nextTab);

    LOG(CERR) << endl;
}

float Cell::get_pin_capacitance(const string& pin_name){
    if(pins.find(pin_name)==pins.end()){
        LOG(ERROR) << "[Cell][get_pin_capacitance] no such pin " << pin_name << endl;
    }else return pins[pin_name]->get_capacitance();
    return 0;
}

string Cell::get_pin_direction(const string& pin_name){
    if(pins.find(pin_name)==pins.end()){
        LOG(ERROR) << "[Cell][get_pin_direction] no such pin " << pin_name << endl;
    }else return pins[pin_name]->get_direction();
    return "";
}

bool Cell::get_pin_is_clock(const string& pin_name){
    if(pins.find(pin_name)==pins.end()){
        LOG(ERROR) << "[Cell][get_pin_is_clock] no such pin " << pin_name << endl;
    }else return pins[pin_name]->get_is_clock();
    return false;
}

vector<TimingArc*>* Cell::get_pin_TimingArc(const string& pin_name, const string& src){
    if(pins.find(pin_name)==pins.end()){
        LOG(ERROR) << "[Cell][get_pin_TimingArc] no such pin " << pin_name << endl;
    }else return pins[pin_name]->get_TimingArc(src);
    return NULL;
}

vector<TimingArc*>* Cell::get_pin_total_TimingArc(const string& pin_name){
    if(pins.find(pin_name)==pins.end()){
        LOG(ERROR) << "[Cell][get_pin_total_TimingArc] no such pin " << pin_name << endl;
    }else return pins[pin_name]->get_total_TimingArc();
    return NULL;
}

Pin* Cell::get_pin_ptr(const string& pin_name){
    if(pins.find(pin_name)==pins.end()){
        LOG(ERROR) << "[Cell][get_pin_ptr] no such pin " << pin_name << endl;
    }else return pins[pin_name];
    return NULL;
}
