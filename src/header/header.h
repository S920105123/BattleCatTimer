#ifndef HEADER_H
#define HEADER_H

#include <algorithm>
#include <utility>
#include <time.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>

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

const int OUTPUT_PRECISION = 3;

const float UNDEFINED_SLEW[2] = { 987654.0 , -987654.0 };  // Early/late
const float UNDEFINED_RAT[2]  = { -987654.0, 987654.0  };  // Early/late
const float UNDEFINED_AT[2]   = { 987654.0 , -987654.0 };  // Early/late
const float UNDEFINED_SLACK[2] = { 987654.0 , 987654.0  };  // Early/late

const Transition_Type TYPES[2] = { RISE, FALL };
const Mode MODES[2] = { EARLY, LATE };

#endif /* end HEADER_H */
