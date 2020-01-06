
#ifndef TASK3_VAR_H
#define TASK3_VAR_H

#include <string>
using namespace std;

class Var {
    double value;
    string sim;
public:
    Var(string theSim) {
        sim = theSim;
    }
    Var(double val) {
        value = val;
    }
    void setValue(double);
    double getValue();
    string getSim();
    ~ Var() {}
};

#endif //TASK3_VAR_H
