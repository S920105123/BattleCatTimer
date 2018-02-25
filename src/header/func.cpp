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

bool at_worse_than(float at1, float at2, Mode mode) {
	// Always choose worst at
	if (mode == EARLY) return at1 < at2;
	return at1 > at2;
}

bool slew_worse_than(float s1, float s2, Mode mode) {
	// Always choose worst at
	if (mode == EARLY) return s1 < s2;
	return s1 > s2;
}

bool rat_worse_than(float r1, float r2, Mode mode) {
	// Always choose worst (most strict) rat
	if (mode == EARLY) return r1 > r2;
	return r2 > r1;
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

const string& get_mode_string(Mode mode){
    static string late_str = string("Late"), early_str = string("Early");
    if(mode==LATE) return late_str;
    else return early_str;
}

const string& get_transition_string(Transition_Type transition){
    static string rise_str = string("Rise"), fall_str = string("Fall");
    if(transition==RISE) return rise_str;
    else return fall_str;
}
