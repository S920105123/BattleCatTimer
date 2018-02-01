#include "lu_table_template.h"

void LuTableTemplate::read(File_Reader &in){
    int level = 1;
    EXPECT(in.next_token(), "{");

    string token;
    while(level){
        token = in.next_token();
        if(token == "}") level--;
        else if(token == "{") level++;
        else if(token=="variable_1"){
            EXPECT(in.next_token(), ":");
            var1 = judge_variable(in.next_token());
        }
        else if(token=="variable_2"){
            EXPECT(in.next_token(), ":");
            var2 = judge_variable(in.next_token());
        }
        else if(token=="index_1"){
            EXPECT(in.next_token(), "(");
            do{
                token = in.next_token();
                if(token==")") break;
                string_to_float(index1, token);
            }while(true);
        }
        else if(token=="index_2"){
            EXPECT(in.next_token(), "(");
            do{
                token = in.next_token();
                if(token==")") break;
                string_to_float(index2, token);
            }while(true);
        }
        else LOG(WARNING) << "[LuTableTemplate][read] unknown keyword: " << token << endl;
    }
}

Lu_Table_Variable LuTableTemplate::judge_variable(const string& name){
    if(name == "total_output_net_capacitance")
        return TOTAL_OUTPUT_NET_CAP;
    else if(name == "input_net_transition")
        return INPUT_NET_TRANSITION;
    else if(name == "related_pin_transition")
        return RELATED_PIN_TRANSIITION;
    else if(name == "constrained_pin_transition")
        return CONSTRAINED_PIN_TRANSITION;
    else{
        LOG(ERROR) << "[LuTableTemplate] can't judge variable: " << name << endl;
        return UNDEFINED_LU_TABLE_VARIABLE;
    }
}

void LuTableTemplate::set_name(const string& name){
    label_name = name;
}

string LuTableTemplate::get_variable_string(Lu_Table_Variable var){
    switch (var) {
        case INPUT_NET_TRANSITION:
            return "input_net_transition";
        case TOTAL_OUTPUT_NET_CAP:
            return "total_output_net_cap";
        case CONSTRAINED_PIN_TRANSITION:
            return "constrained_pin_transition";
        case RELATED_PIN_TRANSIITION:
            return "related_pin_transiition";
        default:
            return "undefined_lu_table_variable";
    }
}

void LuTableTemplate::print(const string &tab){
    LOG(CERR) << tab << "LuTableTemplate: " << label_name << endl;
    LOG(CERR) << tab << "    - variable1 : " << get_variable_string(var1) << endl;
    LOG(CERR) << tab << "    - variable2 : " << get_variable_string(var2) << endl;

    LOG(CERR) << tab << "    - index1 :" ;
    for(auto f:index1) LOG(CERR) << f << " "; cout << endl;

    LOG(CERR) << tab << "    - index2 :" ;
    for(auto f:index2) LOG(CERR) << f << " "; cout << endl;
}

Lu_Table_Variable LuTableTemplate::get_var1(){
    return var1;
}

Lu_Table_Variable LuTableTemplate::get_var2(){
    return var2;
}

#ifdef TEST_LU_TABLE_TEMPLAT

int main()
{
    string filename = "unit_test\\lu_table_template.lib";
    File_Reader in;
    in.open(filename);

    string token, name;
    do{
        token = in.next_token();
        if(token=="lu_table_template"){
            EXPECT(in.next_token(), "(");
            name = in.next_token();
            EXPECT(in.next_token(), ")");
            LuTableTemplate *table = new LuTableTemplate();
            table->set_name(name);
            table->read(in);
            cout << " ------- get " << name <<  " ------- \n";
            table->print();
        }
    }while(token.size());

    Logger::create()->~Logger();
}

#endif /* end TEST_LU_TABLE_TEMPLAT */
