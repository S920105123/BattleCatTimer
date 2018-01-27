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

            cell_rise_table = new TimingTable();
            cell_rise_table->read(in);
            cell_rise_table->set_name(label_name);
        }
        else if(token=="cell_fall"){
            EXPECT(in.next_token(), "(");
            label_name = in.next_token();
            EXPECT(in.next_token(), ")");

            cell_fall_table= new TimingTable();
            cell_fall_table->read(in);
            cell_fall_table->set_name(label_name);
        }
        else if(token=="rise_transition"){
            EXPECT(in.next_token(), "(");
            label_name = in.next_token();
            EXPECT(in.next_token(), ")");

            rise_transition_table = new TimingTable();
            rise_transition_table->read(in);
            rise_transition_table->set_name(label_name);
        }
        else if(token=="fall_transition"){
            EXPECT(in.next_token(), "(");
            label_name = in.next_token();
            EXPECT(in.next_token(), ")");

            fall_transition_table = new TimingTable();
            fall_transition_table->read(in);
            fall_transition_table->set_name(label_name);
        }
        else{
            LOG(WARNING) << "[TimingArc][open] unknown keyword: " << token << endl;
        }
    }
}

string TimingArc::get_related_pin(){
    return related_pin;
}

void TimingArc::set_timing_sense(string val){
    if(val=="negative_unate") timing_sense = NEGATIVE_UNATE;
    else if(val=="positive_unate") timing_sense = POSITIVE_UNATE;
    else if(val=="non_unate") timing_sense = NON_UNATE;
    else{
        LOG(ERROR) << "[TimingArc] can't juge timing sense : " << val << endl;
        timing_sense = UNDEFINED_TIMING_SENSE;
    }
}

void TimingArc::set_timing_type(string val){
    if(val=="rising_edge") timing_type = RISING_EDGE;
    else if(val=="falling_edge") timing_type = FALLING_EDGE;
    else if(val=="combinational") timing_type = COMBINATINAL;
    else{
        LOG(ERROR) << "[TimingArc] can't juge timing type: " << val << endl;
        timing_type = COMBINATINAL;
    }
}

string TimingArc::get_timing_type_string(){
    switch (timing_type) {
        case RISING_EDGE: return "rising_edge";
        case FALLING_EDGE: return "falling_edge";
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
            return "undefined_timing_sense";
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
            TimingArc * arc = new TimingArc();
            arc->read(in);
            cout << " ------- get " << arc->get_related_pin() <<  " ------- \n";
            arc->print();
        }
    }while(token.size());

    Logger::create()->~Logger();
}

#endif /* end TEST_TIMING_ARC */
