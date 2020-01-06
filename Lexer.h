
#ifndef TASK3_LEXER_H
#define TASK3_LEXER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <regex>
#include "Expression.h"

using namespace std;

class Lexer {
    vector<string> tokens;
    void funcTokens(string str);
    string varTokens(string str,string symbol);
    void conditionTokens(string str, string condition);
    string withoutSpaces(string line);
    string spacesInText(string line);
public:
    Lexer();
    vector<string> lexingFile(string const &fileName);

};

#endif //TASK3_LEXER_H
