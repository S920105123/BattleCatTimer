#ifndef HEADER_H
#define HEADER_H

#include <algorithm>
#include <utility>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

#include <string>
#include <vector>
#include <queue>
#include <stack>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <map>
#include <tuple>

#include <ctime>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cctype>

#include "func.h"
#include "enum.h"

/* I/O */
using std::cin;
using std::cout;
using std::istream;
using std::ostream;
using std::ifstream;
using std::ofstream;

/* container */
using std::string;
using std::vector;
using std::map;
using std::unordered_map;
using std::queue;
using std::stack;
using std::priority_queue;
using std::pair;
using std::tuple;
using std::tie;

/*  math */
using std::max;
using std::min;
using std::sqrt;
using std::swap;

const float UNDEFINED_SLEW[2] = { 987654.0 , -987654.0 };  // Early/late
const float UNDEFINED_RAT[2]  = { -987654.0, 987654.0  };  // Early/late
const float UNDEFINED_AT[2]   = { 987654.0 , -987654.0 };  // Early/late

#endif /* end HEADER_H */
