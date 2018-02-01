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
