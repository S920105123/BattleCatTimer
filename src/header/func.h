#ifndef FUNC_H
#define FUNC_H

#include <sstream>
#include <cstring>
#include <vector>
#include "enum.h"

using std::stringstream;
using std::string;
using std::vector;

void string_to_float(vector<float>&, const string &val);
string cell_pin_concat(const string &cell, const string &pin); // "ABC","123" -> "ABC:123"
bool is_prefix(const string &s, const string& prefix);
bool isfloat(const string& s);
string get_mode_string(Mode mode);
string get_transition_string(Transition_Type transition);

#endif /* end FUNC_H */
