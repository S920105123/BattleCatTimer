#include "timing_table.h"

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

void TimingTable::set_name(string name){
    label_name = name;
}

int TimingTable::getValuesIndex(int i,int j){
    return i*index2.size() + j;
}

void TimingTable::print(const string &tab){
    LOG(CERR) << tab << " TimingTable : " << label_name << endl;

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

    string token, name;
    do{
        token = in.next_token();
        if(token=="cell_rise"){
            EXPECT(in.next_token(), "(");
            name = in.next_token();
            EXPECT(in.next_token(), ")");
            TimingTable* table = new TimingTable();
            table->set_name(name);
            table->read(in);
            cout << " ------- get " << name <<  " ------- \n";
            table->print();
        }
    }while(token.size());

    Logger::create()->~Logger();

}

#endif /* end TEST_TIMING_TABLE */
