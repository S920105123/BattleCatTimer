#ifndef FUNC_H
#define FUNC_H

#include <sstream>
#include <cstring>
#include <vector>

using std::stringstream;
using std::string;
using std::vector;

float string_to_float(const string& val);
void string_to_float(vector<float>&, const string& val);

#endif /* end FUNC_H */
