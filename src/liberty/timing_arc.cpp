// #include "timing_arc.h"
#include "liberty.h"

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

            cell_rise_table = new TimingTable(cell_lib, "cell_rise");
            cell_rise_table->set_name(label_name);
            cell_rise_table->read(in);
        }
        else if(token=="cell_fall"){
            EXPECT(in.next_token(), "(");
            label_name = in.next_token();
            EXPECT(in.next_token(), ")");

            cell_fall_table= new TimingTable(cell_lib, "cell_fall");
            cell_fall_table->set_name(label_name);
            cell_fall_table->read(in);
        }
        else if(token=="rise_transition"){
            EXPECT(in.next_token(), "(");
            label_name = in.next_token();
            EXPECT(in.next_token(), ")");

            rise_transition_table = new TimingTable(cell_lib, "rise_transition");
            rise_transition_table->set_name(label_name);
            rise_transition_table->read(in);
        }
        else if(token=="fall_transition"){
            EXPECT(in.next_token(), "(");
            label_name = in.next_token();
            EXPECT(in.next_token(), ")");

            fall_transition_table = new TimingTable(cell_lib, "fall_transition");
            fall_transition_table->set_name(label_name);
            fall_transition_table->read(in);
        }
        else if(token=="rise_constraint"){
            EXPECT(in.next_token(), "(");
            label_name = in.next_token();
            EXPECT(in.next_token(), ")");

            rise_constraint_table = new TimingTable(cell_lib, "rise_constraint");
            rise_constraint_table->set_name(label_name);
            rise_constraint_table->read(in);
        }
        else if(token=="fall_constraint"){
            EXPECT(in.next_token(), "(");
            label_name = in.next_token();
            EXPECT(in.next_token(), ")");

            fall_constraint_table = new TimingTable(cell_lib, "fall_constraint");
            fall_constraint_table->set_name(label_name);
            fall_constraint_table->read(in);
        }
        else{
            LOG(WARNING) << "[TimingArc][open] unknown keyword: " << token << '\n';
        }
    }
}

const string& TimingArc::get_related_pin(){
    return related_pin;
}

void TimingArc::set_timing_sense(const string& val){
    if(val=="negative_unate") timing_sense = NEGATIVE_UNATE;
    else if(val=="positive_unate") timing_sense = POSITIVE_UNATE;
    else if(val=="non_unate") timing_sense = NON_UNATE;
    else{
        LOG(ERROR) << "[TimingArc] can't juge timing sense : " << val << '\n';
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
        LOG(ERROR) << "[TimingArc] can't juge timing type: " << val << '\n';
        timing_type = COMBINATINAL;
    }
}

void TimingArc::set_cell_ptr(Cell *_cell){
    cell = _cell;
}

string TimingArc::get_timing_type_string(){
    switch (timing_type) {
        case RISING_EDGE:  return "rising_edge";
        case FALLING_EDGE: return "falling_edge";
        case SETUP_RISING: return "setup_rising";
        case HOLD_RISING:  return "hold_rising";
        case SETUP_FALLING:
            LOG(WARNING) << "appear setup_falling\n";
            return "setup_falling";
        case HOLD_FALLING:
            LOG(WARNING) << "appear hold_falling\n";
            return "hold_falling";
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

float TimingArc::get_delay(
    Transition_Type from, Transition_Type to, float input_val, float output_val){
    string at = cell->get_type_name() + " " + related_pin + " to " + to_pin;
    if(!is_transition_defined(from, to)){
        LOG(CERR) << "[TimingArc][get_delay] wrong transition arc from = "
        << from << ", to = " << to <<  " at " << at << '\n';
    }
    if(cell_rise_table==NULL){
        LOG(ERROR) << "[TimingArc][get_delay] cell_rise_table==NULL, at " << at << '\n';
        return 0;
    }
    if(cell_fall_table==NULL){
        LOG(ERROR) << "[TimingArc][get_delay] cell_rise_table==NULL, at " << at << '\n';
        return 0;
    }
    switch(timing_sense){
        case POSITIVE_UNATE:
            if(from==RISE) return cell_rise_table->get_value(input_val, output_val);
            else return cell_fall_table->get_value(input_val, output_val);
            break;
        case NEGATIVE_UNATE:
            if(from==FALL) return cell_rise_table->get_value(input_val, output_val);
            else return cell_fall_table->get_value(input_val, output_val);
            break;
        case NON_UNATE:
            if(to==FALL) return cell_fall_table->get_value(input_val, output_val);
            else return cell_rise_table->get_value(input_val, output_val);
            break;
        case UNDEFINED_TIMING_SENSE:
            LOG(ERROR) << "[TimingArc][get_delay] undefined timing sense.\n";
            return 0;
            break;
    }
    return 0;
}

float TimingArc::get_slew(
    Transition_Type from, Transition_Type to, float input_val, float output_val){
    string at = cell->get_type_name() + " " + related_pin + " to " + to_pin;
    if(!is_transition_defined(from, to)){
        LOG(CERR) << "[TimingArc][get_slew] wrong transition arc from = "
        << from << ", to = " << to <<  " at " << at << '\n';
    }
    if(fall_transition_table==NULL){
        LOG(ERROR) << "[TimingArc][get_slew] fall_transition_table==NULL, at " << at << '\n';
        return 0;
    }
    if(rise_transition_table==NULL){
        LOG(ERROR) << "[TimingArc][get_slew] rise_transition_table==NULL, at " << at << '\n';
        return 0;
    }
    switch(timing_sense){
        case POSITIVE_UNATE:
            if(from==RISE) return rise_transition_table->get_value(input_val, output_val);
            else return fall_transition_table->get_value(input_val, output_val);
            break;
        case NEGATIVE_UNATE:
            if(from==FALL) return rise_transition_table->get_value(input_val, output_val);
            else return fall_transition_table->get_value(input_val, output_val);
            break;
        case NON_UNATE:
            if(to==FALL) return fall_transition_table->get_value(input_val, output_val);
            else return rise_transition_table->get_value(input_val, output_val);
            break;
        case UNDEFINED_TIMING_SENSE:
            LOG(ERROR) << "[TimingArc][get_slew] undefined timing sense.\n";
            return 0;
            break;
    }
    return 0;
}

float TimingArc::get_constraint(
    Transition_Type from, Transition_Type to, float input_val, float output_val){
    string at = cell->get_type_name() + " " + related_pin + " to " + to_pin;
    if(!is_constraint()){
        LOG(CERR) << "[TimingArc][get_constraint] isn't constraint arc at " << at << '\n';
        return 0;
    }
    if(!is_transition_defined(from, to)){
        LOG(CERR) << "[TimingArc][get_constraint] wrong transition arc from = "
        << from << ", to = " << to <<  " at " << at << '\n';
        return 0;
    }
    if(fall_constraint_table==NULL){
        LOG(ERROR) << "[TimingArc][get_constraint] fall_constraint_table==NULL, at " << at << '\n';
        return 0;
    }
    if(rise_constraint_table==NULL){
        LOG(ERROR) << "[TimingArc][get_constraint] rise_constraint_table==NULL, at " << at << '\n';
        return 0;
    }
    switch(timing_sense){
        case POSITIVE_UNATE:
        case NEGATIVE_UNATE:
        case NON_UNATE:
            LOG(WARNING) << "[TimingArc][get_constraint] check timing_sense at " << at << '\n';
            return 0;
            break;
        case UNDEFINED_TIMING_SENSE:
            break;
    }

    if(to==RISE) return rise_constraint_table->get_value(input_val, output_val);
    else return fall_constraint_table->get_value(input_val, output_val);
}

float TimingArc::get_delay_constant(Transition_Type from, Transition_Type to){
    string at = cell->get_type_name() + " " + related_pin + " to " + to_pin;
    if(!is_transition_defined(from, to)){
        LOG(CERR) << "[TimingArc][get_delay_constant] wrong transition arc from = "
        << from << ", to = " << to <<  " at " << at << '\n';
    }
    if(cell_rise_table==NULL){
        LOG(ERROR) << "[TimingArc][get_delay_constant] cell_rise_table==NULL, at " << at << '\n';
        return 0;
    }
    if(cell_fall_table==NULL){
        LOG(ERROR) << "[TimingArc][get_delay_constant] cell_rise_table==NULL, at " << at << '\n';
        return 0;
    }
    switch(timing_sense){
        case POSITIVE_UNATE:
            if(from==RISE) return cell_rise_table->get_value_constant();
            else return cell_fall_table->get_value_constant();
            break;
        case NEGATIVE_UNATE:
            if(from==FALL) return cell_rise_table->get_value_constant();
            else return cell_fall_table->get_value_constant();
            break;
        case NON_UNATE:
            if(to==FALL) return cell_fall_table->get_value_constant();
            else return cell_rise_table->get_value_constant();
            break;
        case UNDEFINED_TIMING_SENSE:
            LOG(ERROR) << "[TimingArc][get_delay_constant] undefined timing sense.\n";
            return 0;
            break;
    }
    return 0;
}

float TimingArc::get_slew_constant(Transition_Type from, Transition_Type to){
    string at = cell->get_type_name() + " " + related_pin + " to " + to_pin;
    if(!is_transition_defined(from, to)){
        LOG(CERR) << "[TimingArc][get_slew_constant] wrong transition arc from = "
        << from << ", to = " << to <<  " at " << at << '\n';
    }
    if(fall_transition_table==NULL){
        LOG(ERROR) << "[TimingArc][get_slew_constant] fall_transition_table==NULL, at " << at << '\n';
        return 0;
    }
    if(rise_transition_table==NULL){
        LOG(ERROR) << "[TimingArc][get_slew_constant] rise_transition_table==NULL, at " << at << '\n';
        return 0;
    }
    switch(timing_sense){
        case POSITIVE_UNATE:
            if(from==RISE) return rise_transition_table->get_value_constant();
            else return fall_transition_table->get_value_constant();
            break;
        case NEGATIVE_UNATE:
            if(from==FALL) return rise_transition_table->get_value_constant();
            else return fall_transition_table->get_value_constant();
            break;
        case NON_UNATE:
            if(to==FALL) return fall_transition_table->get_value_constant();
            else return rise_transition_table->get_value_constant();
            break;
        case UNDEFINED_TIMING_SENSE:
            LOG(ERROR) << "[TimingArc][get_slew_constant] undefined timing sense.\n";
            return 0;
            break;
    }
    return 0;
}

float TimingArc::get_constraint_constant(Transition_Type from, Transition_Type to){
    string at = cell->get_type_name() + " " + related_pin + " to " + to_pin;
    if(!is_constraint()){
        LOG(CERR) << "[TimingArc][get_constraint_constant] isn't constraint arc at " << at << '\n';
        return 0;
    }
    if(!is_transition_defined(from, to)){
        LOG(CERR) << "[TimingArc][get_constraint_constant] wrong transition arc from = "
        << from << ", to = " << to <<  " at " << at << '\n';
        return 0;
    }
    if(fall_constraint_table==NULL){
        LOG(ERROR) << "[TimingArc][get_constraint_constant] fall_constraint_table==NULL, at " << at << '\n';
        return 0;
    }
    if(rise_constraint_table==NULL){
        LOG(ERROR) << "[TimingArc][get_constraint_constant] rise_constraint_table==NULL, at " << at << '\n';
        return 0;
    }
    switch(timing_sense){
        case POSITIVE_UNATE:
        case NEGATIVE_UNATE:
        case NON_UNATE:
            LOG(WARNING) << "[TimingArc][get_constraint_constant] check timing_sense at " << at << '\n';
            return 0;
            break;
        case UNDEFINED_TIMING_SENSE:
            break;
    }

    if(to==RISE){
        cout << rise_constraint_table->get_value_constant() << '\n';
        return rise_constraint_table->get_value_constant();
    }
    else return fall_constraint_table->get_value_constant();
}

bool TimingArc::is_constraint(){
    switch(timing_type){
        case HOLD_RISING:
        case HOLD_FALLING:
        case SETUP_RISING:
        case SETUP_FALLING:
            return true;
        default:
            return false;
    }
}

bool TimingArc::is_transition_defined(Transition_Type from, Transition_Type to){
    if(is_rising_triggered()  and from!=RISE) return false;
    if(is_falling_triggered() and from!=FALL) return false;

    if(timing_sense==POSITIVE_UNATE and from!=to) return false;
    if(timing_sense==NEGATIVE_UNATE and from==to) return false;

    return true;
}

bool TimingArc::is_falling_triggered(){
    switch(timing_type){
        case SETUP_FALLING:
        case HOLD_FALLING:
        case FALLING_EDGE:
            return true;
        default:
            return false;
    }
}

bool TimingArc::is_rising_triggered(){
    switch(timing_type){
        case SETUP_RISING:
        case HOLD_RISING:
        case RISING_EDGE:
            return true;
        default:
            return false;
    }
}

void TimingArc::print(const string &tab){
    LOG(CERR) << tab << "related pin : " <<  related_pin << '\n';
    LOG(CERR) << tab << "timing_sense : " << get_timing_sense_string() << '\n';
    LOG(CERR) << tab << "timing_type : " << get_timing_type_string() << '\n';

    string nextTab = tab + "    ";
    LOG(CERR) << tab << "cell_rise_table : " << '\n';
    if(cell_rise_table) cell_rise_table->print(nextTab);

    LOG(CERR) << tab << "cell_fall_table : " << '\n';
    if(cell_fall_table) cell_fall_table->print(nextTab);

    LOG(CERR) << tab << "rise_transition_table : " << '\n';
    if(rise_transition_table) rise_transition_table->print(nextTab);

    LOG(CERR) << tab << "fall_transition_table: " << '\n';
    if(fall_transition_table) fall_transition_table->print(nextTab);

    LOG(CERR) << tab << "rise_constraint_table: " << '\n';
    if(rise_constraint_table) rise_constraint_table->print(nextTab);

    LOG(CERR) << tab << "fall_constraint_table: " << '\n';
    if(fall_constraint_table) fall_constraint_table->print(nextTab);
    cout << '\n';
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
            TimingArc * arc = new TimingArc(NULL){;
            arc->read(in);
            cout << " ------- get " << arc->get_related_pin() <<  " ------- \n";
            arc->print();
        }
    }while(token.size());

    Logger::create()->~Logger();
}

#endif /* end TEST_TIMING_ARC */
