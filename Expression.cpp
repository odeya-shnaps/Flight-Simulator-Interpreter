#include <string>
#include "Expression.h"
#include <queue>
#include <stack>
#include <regex>


Variable::Variable(string thename, double thevalue) {
    this->name = thename;
    this->value = thevalue;
}

Variable& Variable::operator++(){
    this->value +=1;
    return *this;
}

Variable& Variable:: operator--() {
    this->value -=1;
    return *this;
}

Variable& Variable:: operator+=(double num) {
    this->value = this->value + num;
    return *this;
}

Variable& Variable:: operator-=(double num) {
    this->value = this->value - num;
    return *this;
}

Variable& Variable::operator++(int){
    this->value = this->value+1;
    return *this;
}

Variable& Variable::operator--(int){
    this->value = value-1;
    return *this;
}

double Variable::calculate() {return this->value;}

Value:: Value(double theval):value(theval) {}

double Value::calculate() {return this->value;}

Plus::Plus(Expression* theleft, Expression* theright) {
    this->left = theleft;
    this->right = theright;
}

Plus::~Plus() {
    if (left != nullptr) {
        delete left;
    }
    if (right != nullptr) {
        delete right;
    }
}

Minus::Minus(Expression *theleft, Expression *theright) {
    this->left = theleft;
    this->right = theright;
}

Minus::~Minus() {
    if (left != nullptr) {
        delete left;
    }
    if (right != nullptr) {
        delete right;
    }
}

double Plus::calculate() { return left->calculate() + right->calculate(); }

double Minus::calculate() { return left->calculate() - right->calculate(); }

Mul::Mul(Expression *theleft, Expression *theright) {
    this->left = theleft;
    this->right = theright;
}

double Mul::calculate() { return left->calculate() * right->calculate(); }
Div::Div(Expression *theleft, Expression *theright) {
    this->left = theleft;
    this->right = theright;
}

Mul::~Mul() {
    if (left != nullptr) {
        delete left;
    }
    if (right != nullptr) {
        delete right;
    }
}

double Div::calculate() {
    double denominator = right->calculate();
    if (denominator == 0) {
        throw "division by zero";
    }
    return left->calculate() / denominator;
}

Div::~Div() {
    if (left != nullptr) {
        delete left;
    }
    if (right != nullptr) {
        delete right;
    }
}
UPlus::UPlus(Expression *theexp) {this->expression = theexp;}

double UPlus::calculate() {return expression->calculate();}

UPlus::~UPlus() {
    if (expression != nullptr) {
        delete expression;
    }
}

UMinus::UMinus(Expression *theexp) {this->expression = theexp;}

double UMinus::calculate() {return -1*expression->calculate();}

UMinus::~UMinus() {
    if (expression != nullptr) {
        delete expression;
    }
}

Expression* Interpreter::interpret(string str) {
    queue <string> outputQueue;
    stack <string> operatorStack;
    stack <Expression*> exprStack;

    unsigned int i;
    string num;
    bool flag = false;
    for (i=0; i<str.length(); i++) {
        if (((str[i] >=40) && (str[i] <=43)) || (str[i] == 45)|| (str[i] == 47)){
            if (flag) { //need to add a number to queue
                outputQueue.push(num);
                num.clear();
                flag = false;
            }
        } else {
            flag = true;
            num.append(str, i, 1);
        }

        if ((i>0) && ((str[i]==42) ||(str[i] ==43)|| (str[i] == 45)|| (str[i] == 47))
            &&((str[i-1]==42) ||(str[i-1] ==43)|| (str[i-1] == 45)|| (str[i-1] == 47))) {
            throw "invalid expression";
        }

        if (str[i] == 40) { //was "("
            operatorStack.push("(");
        } else if ((str[i] == 43) && ((i == 0) || (str[i-1] == 40))) {//unary +
            operatorStack.push("1+"); //sign that unary + and not binary
        } else if ((str[i] == 45) && ((i == 0) || (str[i-1] == 40))) {
            operatorStack.push("1-");
        } else if ((str[i] == 43) || (str[i] == 45)) {
            while ((!operatorStack.empty()) && (operatorStack.top() != "(")) {
                outputQueue.push(operatorStack.top());
                operatorStack.pop();
            }
            if (str[i] == 43) {operatorStack.push("+");}
            else {operatorStack.push("-");}
        } else if ((str[i] == 42) || (str[i] == 47)) {
            while ((!operatorStack.empty()) && (operatorStack.top() != "(") && ((operatorStack.top() == "*")|| (operatorStack.top() == "/"))) {
                outputQueue.push(operatorStack.top());
                operatorStack.pop();
            }
            if (str[i] == 42) {operatorStack.push("*");}
            else {operatorStack.push("/");}
        } else if (str[i] == 41) {
            while ((!operatorStack.empty()) && (operatorStack.top() != "(")) {
                outputQueue.push(operatorStack.top());
                operatorStack.pop();
            }
            if (operatorStack.empty()) {throw "Invalid expression";}
            else if (operatorStack.top() == "(") {operatorStack.pop();}
        }
    } //end of for loop
    if (flag) { //at the end of expression there was a number
        outputQueue.push(num);
    }
    while (!operatorStack.empty()) {
        if (operatorStack.top() == "(") {throw "Invalid Expression";}
        outputQueue.push(operatorStack.top());
        operatorStack.pop();
    }

    return postfixToExpression(outputQueue);

}

Expression* Interpreter::postfixToExpression(queue <string> &postfixExpression) {
    stack <Expression*> exprStack;
    Expression* exp1, *exp2;
    while (!postfixExpression.empty()) {
        string token = postfixExpression.front();
        postfixExpression.pop();
        unordered_map<string, Var>::iterator itr = varsToSim->find(token);
        unordered_map<string, int>::iterator it2 = varsFromSim->find(token);
        if (itr != varsToSim->end()) { //token is in map of variables
            exprStack.push(new Variable(token, itr->second.getValue()));
        } else if (it2!= varsFromSim->end()) { //token is a variable from simulator
            int pos = it2->second;
            m->lock();
            exprStack.push(new Variable(token, (*simVals)[pos]));
            m->unlock();
        } else if (isDoubleNumber(token)) { //token is a number
            exprStack.push(new Value(stod(token)));
        } else if ((token == "+")|| (token =="-")|| (token == "*") ||(token == "/")) { //binary expression
            exp2 = exprStack.top();
            exprStack.pop();
            exp1 = exprStack.top();
            exprStack.pop();
            if (token == "+") {
                exprStack.push(new Plus(exp1, exp2));
            } else if (token =="-") {
                exprStack.push(new Minus(exp1, exp2));
            } else if (token =="*") {
                exprStack.push(new Mul(exp1, exp2));
            } else {
                exprStack.push(new Div(exp1, exp2));
            }
        } else if (token == "1+") {
            exp1 = exprStack.top();
            exprStack.pop();
            exprStack.push(new UPlus(exp1));
        } else if (token == "1-") {
            exp1 = exprStack.top();
            exprStack.pop();
            exprStack.push(new UMinus(exp1));
        } else { //token is not valid
            throw "unassigned variable or invalid number";
        }
    }
    if (exprStack.size() != 1) {
        throw "problem creating expression";
    } else {
        return exprStack.top();
    }

}

bool Interpreter::isDoubleNumber(const string &str) {
    regex reg("^(-?)(0|([1-9][0-9]*))(\\.[0-9]+)?$");
    return regex_match(str, reg);
}

bool Interpreter::isGoodVariableName(const string &str) {
    if (((str[0] >=48) && (str[0] <= 57))||(str[0] == 95)) {
        return false;
    } else {
        regex reg("^(([A-Z|[a-z]|_|[0-9])+)$");
        return regex_match(str, reg);
    }
}

void Interpreter::setM(mutex *mut) {
    m = mut;
}

