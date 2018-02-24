// #include "timing_table.h"
#include "liberty.h"

void TimingTable::read(File_Reader &in){

    string token;

    int level = 1;
    EXPECT(in.next_token(), "{");

    while(level){
        token = in.next_token();
        if(token == "}") level--;
        else if(token == "{") level++;
        else if(token == "values"){
            EXPECT(in.next_token(), "(");
            do{
                token = in.next_token();
                if(token==")") break;
                else if(token!="\\") string_to_float(values, token);
            }while(true);
        }
        else if(token == "index_1"){
            EXPECT(in.next_token(), "(");
            do{
                token = in.next_token();
                if(token==")") break;
                else string_to_float(index1, token);
            }while(true);
        }
        else if(token == "index_2"){
            EXPECT(in.next_token(), "(");
            do{
                token = in.next_token();
                if(token==")") break;
                else string_to_float(index2, token);
            }while(true);
        }
        else{
            LOG(WARNING) << "[TimingTable][read] unknown keyword: " << token << endl;
        }
    }
}

void TimingTable::set_name(const string& name){
    label_name = name;
}

int TimingTable::getValuesIndex(int i,int j){
    return i*index2.size() + j;
}

// idx: input_val : var1, idy: output_val : var2
float TimingTable::get_value(float input_val,float output_val){
    #ifdef SHOW_TIMINGTABLE
        if(values.size()==1 and index1.size()==0 and index2.size()==0) return values[0];
        LOG(CERR) << "input_val: " << input_val << ", output_val: " << output_val << endl;
        LOG(CERR) << "Using table: \n";
        print("  ");
    #endif
    if(label_name=="scalar") return values[0];
    LuTableTemplate* tu = cell_lib->get_table_template(label_name);
    /* INPUT_NET_TRANSITION(input_val) and TOTAL_OUTPUT_NET_CAP(output_val)  or
       CONSTRAINED_PIN_TRANSITION(data; output_val) and RELATED_PIN_TRANSIITION(clock: input_val)

       input_val  : input_slew, or clock slew
       output_val : cload      or data slew
       */
    if(tu->get_var1()==TOTAL_OUTPUT_NET_CAP or tu->get_var1()==CONSTRAINED_PIN_TRANSITION){
        std::swap(input_val, output_val);
        // LOG(WARNING) << "[TimingTable][get_value] table: " << label_name << " check its variable order.";
    }

    int idx = std::lower_bound(index1.begin(), index1.end(), input_val) - index1.begin();
    int idy = std::lower_bound(index2.begin(), index2.end(), output_val) - index2.begin();

    idx = max(1, min(idx, (int)index1.size()-1));
    idy = max(1, min(idy, (int)index2.size()-1));

    if(index1.size()==1){
        return liner_polation(values[getValuesIndex(0, idy-1)], values[getValuesIndex(0, idy)],
            index2[idy-1], index2[idy], output_val);
    }
    else if(index2.size()==1){
        return liner_polation(values[getValuesIndex(idx-1, 0)], values[getValuesIndex(idx, 0)],
            index1[idx-1], index2[idx], input_val);
    }
    else{
        float val1, val2;
        val1 = liner_polation(values[getValuesIndex(idx-1, idy-1)], values[getValuesIndex(idx-1, idy)],
            index2[idy-1], index2[idy], output_val);
        val2 = liner_polation(values[getValuesIndex(idx, idy-1)], values[getValuesIndex(idx, idy)],
            index2[idy-1], index2[idy], output_val);
        return liner_polation(val1, val2, index1[idx-1], index1[idx], input_val);
    }
}

/*
    x    indx1   x    indx2   x
          v1           v2
*/
float TimingTable::liner_polation(float v1, float v2, float indx1, float indx2, float x)
{
    // cout << v1 << " " << v2 << " " << indx1 << " " << indx2 << " " << x << endl;
    if(x==indx1) return v1;
    else if(x==indx2) return v2;
    else if(indx1<x and x<indx2) return v1 + (x-indx1)*(v2-v1)/(indx2-indx1);
    else if(x<indx1){
        // LOG(WARNING) << "[TimingTable][liner_polation] using extra polation.";
        return v1 - (indx1-x)*(v2-v1)/(indx2-indx1);
    }
    else if(x>indx2){
        // LOG(WARNING) << "[TimingTable][liner_polation] using extra polation.";
        return v2 + (x-indx2)*(v2-v1)/(indx2-indx1);
    }
    else ASSERT_NOT_REACHED();
    return 0;
}

float TimingTable::get_value_constant(int precision){
    int base = std::pow(10, precision);
    if(values.size()) return (float)( (int)(values[0]*base*1.0 +0.55)/(1.0*base));
    else return 0;
}

void TimingTable::print(const string &tab){
    LOG(CERR) << tab << " TimingTable : " << table_type << ", " << label_name << endl;

    LOG(CERR) << tab << "  - index1: ";
    for(auto f:index1) LOG(CERR) << f << ", "; LOG(CERR) << endl;

    LOG(CERR) << tab << "  - index2: ";
    for(auto f:index2) LOG(CERR) << f << ", "; LOG(CERR) << endl;

    LOG(CERR) << tab << "  - values: \n";
    for(size_t i=0; i<index1.size(); i++){
        LOG(CERR) << tab;
        for(size_t j=0; j<index2.size(); j++){
            LOG(CERR) << values[getValuesIndex(i,j)] << ", ";
        }
        LOG(CERR) << endl;
    }
    if(label_name == "scalar" && values.size()) LOG(CERR) << tab << values[0] << endl;
}

#ifdef TEST_TIMING_TABLE

int main()
{
    string filename = "unit_test\\timing_table.lib";
    File_Reader in;
    in.open(filename);

    TimingTable* table;
    string token, name;
    do{
        token = in.next_token();
        if(token=="cell_rise"){
            EXPECT(in.next_token(), "(");
            name = in.next_token();
            EXPECT(in.next_token(), ")");
            table = new TimingTable(NULL);
            table->set_name(name);
            table->read(in);
            cout << " ------- get " << name <<  " ------- \n";
            table->print();
        }
    }while(token.size());

    float x, y;
    do{
        cout << "enter input cload and slew: ";
        cin >> x >> y ;
        if(x==-1 and y==-1) break;
        cout << "ans = " << table->get_value(x, y) << endl;
    }while(true);
    Logger::create()->~Logger();

}

#endif /* end TEST_TIMING_TABLE */
