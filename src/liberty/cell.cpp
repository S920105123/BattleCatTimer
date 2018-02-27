// #include "cell.h"
#include "liberty.h"

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
        else if(token=="cell_leakage_power"){
            EXPECT(in.next_token(), ":");
            in.next_token();
        }
        else if(token=="area"){
            EXPECT(in.next_token(), ":");
            in.next_token();
        }
        else if(token=="cell_footprint"){
            EXPECT(in.next_token(), ":");
            in.next_token();
        }
        else if(token=="ff" or token=="latch"){
            EXPECT(in.next_token(), "(");
            in.next_token(); in.next_token();
            EXPECT(in.next_token(), ")");
            EXPECT(in.next_token(), "{");
            do{
                token = in.next_token();
                if(token=="}") break;
            }while(true);
        }
        else{
            LOG(WARNING) << "[Cell][read] line: " << in.get_lineno() << ", unknow keyword: " << token << '\n';
        }
    }
}

void Cell::set_name(const string& name){
    type_name = name;
}

void Cell::add_pin(const string& name, Pin* pin){
    if(pins.find(name)==pins.end()){
        pins[name] = pin;
    }else LOG(ERROR) << "[Cell][add_pin] Cell: " << type_name << " " << name << " appear twice.\n";
}

void Cell::print(const string &tab){
    LOG(CERR) << tab << " Cell : " << type_name << '\n';

    string nextTab = tab + "    ";
    for(auto &i : pins) i.second->print(nextTab);

    LOG(CERR) << '\n';
}

float Cell::get_pin_capacitance(const string& pin_name){
    if(pins.find(pin_name)==pins.end()){
        LOG(ERROR) << "[Cell][get_pin_capacitance] Cell: " << type_name << " no such pin " << pin_name << '\n';
    }else return pins[pin_name]->get_capacitance();
    return 0;
}

Direction_type Cell::get_pin_direction(const string& pin_name){
    if(pins.find(pin_name)==pins.end()){
        LOG(ERROR) << "[Cell][get_pin_direction] Cell: " << type_name << " no such pin " << pin_name << '\n';
    }else return pins[pin_name]->get_direction();
    return OUTPUT;
}

bool Cell::get_pin_is_clock(const string& pin_name){
    if(pins.find(pin_name)==pins.end()){
        LOG(ERROR) << "[Cell][get_pin_is_clock] Cell: " << type_name << " no such pin " << pin_name << '\n';
    }else return pins[pin_name]->get_is_clock();
    return false;
}

vector<TimingArc*>* Cell::get_pin_TimingArc(const string& pin_name, const string& src){
    if(pins.find(pin_name)==pins.end()){
        LOG(ERROR) << "[Cell][get_pin_TimingArc] Cell: " << type_name << " no such pin " << pin_name << '\n';
    }else return pins[pin_name]->get_TimingArc(src);
    return NULL;
}

vector<TimingArc*>* Cell::get_pin_total_TimingArc(const string& pin_name){
    if(pins.find(pin_name)==pins.end()){
        LOG(ERROR) << "[Cell][get_pin_total_TimingArc] Cell: " << type_name << " no such pin " << pin_name << '\n';
    }else return pins[pin_name]->get_total_TimingArc();
    return NULL;
}

Pin* Cell::get_pin_ptr(const string& pin_name){
    if(pins.find(pin_name)==pins.end()){
        LOG(ERROR) << "[Cell][get_pin_ptr] Cell: " << type_name << " no such pin " << pin_name << '\n';
    }else return pins[pin_name];
    return NULL;
}
