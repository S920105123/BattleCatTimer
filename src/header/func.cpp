#include "func.h"

float string_to_float(string val){
    stringstream ss(val);
    float res;
    ss>>res;
    return res;
}
