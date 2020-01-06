
#ifndef TASK3_EXPRESSION_H
#define TASK3_EXPRESSION_H

#include <iostream>
#include <string>
#include <iterator>
#include <map>
#include <unordered_map>
#include <queue>
#include <mutex>
#include "Var.h"

using namespace std;

/**
 * Expression Interface
 */
class Expression {

public:
    virtual double calculate() = 0;
    virtual ~Expression() {}
};

class Variable: public Expression {
    string name;
    double value;

public:
    Variable(string, double);
    virtual double calculate();

    Variable& operator++();
    Variable& operator--();
    Variable& operator+=(double);
    Variable& operator-=(double);
    Variable& operator++(int);
    Variable& operator--(int);
};

class Value: public Expression {
    const double value;

public:
    Value(double);
    virtual double calculate();
};

class BinaryOperation: public Expression {
protected:
    Expression *right;
    Expression *left;
};

class Plus: public BinaryOperation {
public:
    Plus(Expression*, Expression*);
    virtual double calculate();
    virtual ~Plus();
};

class Minus: public BinaryOperation {
public:
    Minus(Expression*, Expression*);

    virtual double calculate();
    virtual ~Minus();
};

class Mul:public BinaryOperation {
public:
    Mul(Expression*, Expression*);

    virtual double calculate();
    virtual ~Mul();
};

class Div:public BinaryOperation {
public:
    Div(Expression*, Expression*);

    virtual double calculate();
    virtual ~Div();
};

class UnaryOperator: public Expression {
protected:
    Expression* expression;
};

class UPlus: public UnaryOperator {
public:
    UPlus(Expression*);


    virtual double calculate();
    virtual ~UPlus();
};

class UMinus: public UnaryOperator {
public:
    UMinus(Expression*);

    virtual double calculate();
    virtual ~UMinus();
};

class Interpreter {
    unordered_map<string, Var> *varsToSim;
    unordered_map<string, int> *varsFromSim;
    vector<double> *simVals;
    mutex* m;
    Expression* postfixToExpression(queue <string> &postfixExpression);

public:
    Interpreter (unordered_map<string, Var> *varsToSimulator, unordered_map<string, int> *varsFromSimulator,
                 vector<double> *simulatorVals) {
        varsToSim = varsToSimulator;
        varsFromSim = varsFromSimulator;
        simVals = simulatorVals;
    }
    Expression* interpret(string str);
    bool isDoubleNumber(const string &str);
    bool isGoodVariableName(const string &str);
    void setM(mutex*);
};

#endif //TASK3_EXPRESSION_H
