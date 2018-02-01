#include "timing_arc.h"

void TimingArc::read(File_Reader &in){

    int level = 1;
    EXPECT(in.next_token(), "{");

    string token, label_name;
    while(level){
        token = in.next_token();
        if(token=="}") level--;
        else if(token=="{") level++;
        else if(token=="related_pin"){
            EXPECT(in.next_token(), ":");
            related_pin = in.next_token();
        }
        else if(token=="timing_sense"){
            EXPECT(in.next_token(), ":");
            set_timing_sense( in.next_token() );
        }
        else if(token=="timing_type"){
            EXPECT(in.next_token(), ":");
            set_timing_type( in.next_token() );
        }
        else if(token=="cell_rise"){
            EXPECT(in.next_token(), "(");
            label_name = in.next_token();
            EXPECT(in.next_token(), ")");

            cell_rise_table = new TimingTable(cell_lib);
            cell_rise_table->set_name(label_name);
            cell_rise_table->read(in);
        }
        else if(token=="cell_fall"){
            EXPECT(in.next_token(), "(");
            label_name = in.next_token();
            EXPECT(in.next_token(), ")");

            cell_fall_table= new TimingTable(cell_lib);
            cell_fall_table->set_name(label_name);
            cell_fall_table->read(in);
        }
        else if(token=="rise_transition"){
            EXPECT(in.next_token(), "(");
            label_name = in.next_token();
            EXPECT(in.next_token(), ")");

            rise_transition_table = new TimingTable(cell_lib);
            rise_transition_table->set_name(label_name);
            rise_transition_table->read(in);
        }
        else if(token=="fall_transition"){
            EXPECT(in.next_token(), "(");
            label_name = in.next_token();
            EXPECT(in.next_token(), ")");

            fall_transition_table = new TimingTable(cell_lib);
            fall_transition_table->set_name(label_name);
            fall_transition_table->read(in);
        }
        else if(token=="rise_constraint"){
            EXPECT(in.next_token(), "(");
            label_name = in.next_token();
            EXPECT(in.next_token(), ")");

            rise_constraint_table = new TimingTable(cell_lib);
            rise_constraint_table->set_name(label_name);
            rise_constraint_table->read(in);
        }
        else if(token=="fall_constraint"){
            EXPECT(in.next_token(), "(");
            label_name = in.next_token();
            EXPECT(in.next_token(), ")");

            fall_constraint_table = new TimingTable(cell_lib);
            fall_constraint_table->set_name(label_name);
            fall_constraint_table->read(in);
        }
        else{
            LOG(WARNING) << "[TimingArc][open] unknown keyword: " << token << endl;
        }
    }
}

string TimingArc::get_related_pin(){
    return related_pin;
}

void TimingArc::set_timing_sense(const string& val){
    if(val=="negative_unate") timing_sense = NEGATIVE_UNATE;
    else if(val=="positive_unate") timing_sense = POSITIVE_UNATE;
    else if(val=="non_unate") timing_sense = NON_UNATE;
    else{
        LOG(ERROR) << "[TimingArc] can't juge timing sense : " << val << endl;
        timing_sense = UNDEFINED_TIMING_SENSE;
    }
}

void TimingArc::set_timing_type(const string& val){
    if(val=="rising_edge")       timing_type = RISING_EDGE;
    else if(val=="falling_edge") timing_type = FALLING_EDGE;
    else if(val=="setup_rising") timing_type = SETUP_RISING;
    else if(val=="hold_rising")  timing_type = HOLD_RISING;
    else if(val=="setup_falling") timing_type = SETUP_FALLING;
    else if(val=="hold_falling")  timing_type = HOLD_FALLING;
    else if(val=="combinational")timing_type = COMBINATINAL;
    else if(val=="undefined")    timing_type = UNDEFINED_TIMING_TYPE;
    else{
        LOG(ERROR) << "[TimingArc] can't juge timing type: " << val << endl;
        timing_type = COMBINATINAL;
    }
}

string TimingArc::get_timing_type_string(){
    switch (timing_type) {
        case RISING_EDGE:  return "rising_edge";
        case FALLING_EDGE: return "falling_edge";
        case SETUP_RISING: return "setup_rising";
        case HOLD_RISING:  return "hold_rising";
        case SETUP_FALLING: return "setup_falling";
        case HOLD_FALLING:  return "hold_falling";
        case UNDEFINED_TIMING_TYPE: return "undefined";
        default: return "combinatinal";
    }
}

string TimingArc::get_timing_sense_string(){
    switch (timing_sense) {
        case NEGATIVE_UNATE:
            return "negative_unate";
        case POSITIVE_UNATE:
            return "positive_unate";
        case NON_UNATE:
            return "non_unate";
        default:
            return "undefined";
    }
}

void TimingArc::print(const string &tab){
    LOG(CERR) << tab << "related pin : " <<  related_pin << endl;
    LOG(CERR) << tab << "timing_sense : " << get_timing_sense_string() << endl;
    LOG(CERR) << tab << "timing_type : " << get_timing_type_string() << endl;

    string nextTab = tab + "    ";
    LOG(CERR) << tab << "cell_rise_table : " << endl;
    if(cell_rise_table) cell_rise_table->print(nextTab);

    LOG(CERR) << tab << "cell_fall_table : " << endl;
    if(cell_fall_table) cell_fall_table->print(nextTab);

    LOG(CERR) << tab << "rise_transition_table : " << endl;
    if(rise_transition_table) rise_transition_table->print(nextTab);

    LOG(CERR) << tab << "fall_transition_table: " << endl;
    if(fall_transition_table) fall_transition_table->print(nextTab);

    LOG(CERR) << tab << "rise_constraint_table: " << endl;
    if(rise_constraint_table) rise_constraint_table->print(nextTab);

    LOG(CERR) << tab << "fall_constraint_table: " << endl;
    if(fall_constraint_table) fall_constraint_table->print(nextTab);
    cout << endl;
}

#ifdef TEST_TIMING_ARC

int main()
{
    string filename = "unit_test\\timing_table.lib";
    File_Reader in;
    in.open(filename);

    string token, name;
    do{
        token = in.next_token();
        if(token=="timing"){
            EXPECT(in.next_token(), "(");
            EXPECT(in.next_token(), ")");
            TimingArc * arc = new TimingArc(NULL);
            arc->read(in);
            cout << " ------- get " << arc->get_related_pin() <<  " ------- \n";
            arc->print();
        }
    }while(token.size());

    Logger::create()->~Logger();
}

#endif /* end TEST_TIMING_ARC */
