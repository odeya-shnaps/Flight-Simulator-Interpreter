#include "Var.h"

void Var:: setValue(double num) {
    this->value = num;
}

double Var:: getValue() {
    return this->value;
}

string Var:: getSim() {
    return this->sim;
}
