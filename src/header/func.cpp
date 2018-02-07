#include "func.h"

void string_to_float(vector<float> &to, const string &val){
    const char *beg = val.c_str(), *cur = beg;
	while (cur - beg < (int)val.size()) {
		while (*cur == ',' || !isprint(*cur) || *cur==' ' || *cur=='\n'){
			cur++;
			if (cur - beg >= (int)val.size()) return;
		}
		float f = std::strtof(cur,(char**)&cur);
		to.push_back(f);
	}
}

string cell_pin_concat(const string &cell, const string &pin) {
	// "cell", "pin"  ->  "cell:pin"
	string res=cell;
	res += ':';
	res += pin;
	return res;
}

bool is_prefix(const string& s, const string &prefix){
    if(prefix.size() > s.size()) return false;
    for(size_t i=0; i<prefix.size(); i++){
        if(prefix[i]!=s[i]) return false;
    }
    return true;
}

bool isfloat(const string& s){
    size_t pos;
    try{
        stof(s,&pos);
        return (pos==s.size());
    }
    catch(...){
        return false;
    }
}

string get_mode_string(Mode mode){
    if(mode==LATE) return "Late";
    else return "Early";
}

string get_transition_string(Transition_Type transition){
    if(transition==RISE) return "Rise";
    else return "Fall";
}
