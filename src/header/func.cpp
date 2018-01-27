#include "func.h"

float string_to_float(string val){
    stringstream ss(val);
    float res;
    ss>>res;
    return res;
}

void string_to_float(vector<float> &to, string val){
    string cur;
    for(size_t i=0; i<val.size(); i++){
        if(val[i]=='.' or (val[i]>='0' and val[i]<='9') )
            cur += val[i];
        else if(!cur.empty()){
            to.push_back(string_to_float(cur));
            cur.clear();
        }
    }
    if(!cur.empty()) to.push_back(string_to_float(cur));
}
