#include "Lexer.h"

Lexer::Lexer() {

}
void Lexer::funcTokens(string str) {
    string token, func;
    int flag = 0;
    size_t parentheses = str.find('(');
    token = str.substr(0, parentheses);
    tokens.push_back(token);
    str.erase(0, parentheses+1);
    if (str[str.length()-1] == '{') {
        flag = 1;
        token = str.substr(0, str.length()-2);
    } else {
        token = str.substr(0, str.length()-1);
    }
    std::istringstream tokenStream(token); //separate function parameters
    while (getline(tokenStream, token, ','))
    {
        tokens.push_back(token);
    }
    if (flag) {
        tokens.push_back("{");
    }
}

vector<string> Lexer:: lexingFile(string const &fileName) {
    ifstream readCode;
    string line, token;
    regex empyLine("[ \t]*");
    regex funcName("[A-Za-z]+\\(.*");
    regex initializeVar("var .*");
    regex varByVar(".+=.*");
    regex whileCommand("while .*");
    regex ifCommand("if .*");
    readCode.open(fileName, ios::in);
    if (!readCode) {
        cout << "Unable to open file";
        exit(1); // terminate with error
    }
    while ( getline (readCode,line) )
    {
        if (regex_match(line, empyLine)) {
            continue;
        }
        if (line.find('\"') == string::npos) { //if doesn't have parentheses can erase all spaces in line
            line = withoutSpaces(line);
        } else {
            line = spacesInText(line);
        }
        if (regex_match(line, funcName)) { //command is a function with parameters
            funcTokens(line);
        } else if (regex_match(line, ifCommand)){ //line is if condition
            conditionTokens(line, "if");
        } else if (regex_match(line, whileCommand)) { //line is while loop
            conditionTokens(line, "while");
        } else if (regex_match(line, initializeVar)) {
            tokens.push_back(line.substr(0, 3)); //first word is "var"
            line.erase(0, 4); //erasing "var "
            /*
             * varTokens puts all tokens in vector except sim.. so that needs to be sent to
             * func method to be taken care of.
             */
            if (line.find("->") != string::npos) {
                funcTokens(varTokens(line, "->"));
            } else if (line.find("<-") != string::npos){
                funcTokens(varTokens(line, "<-"));
            } else {
                tokens.push_back(varTokens(line, "="));
            }
        } else if (regex_match(line, varByVar)) {
            tokens.push_back(varTokens(line, "="));
        } else { //line must be } or {
            tokens.push_back(line);
        }
    }
    readCode.close();
    return tokens;
}

void Lexer:: conditionTokens(string str, string condition) {
    tokens.push_back(str.substr(0, condition.length())); //first word is condition word
    str.erase(0, condition.length()+1); //erasing condition word and space after it

    //if have { at the end we will erase it so we can send condition to arithmeticToken to erase spaces
    size_t strLength = str.length();
    if (str[strLength-1] == '{') {
        str.erase(str.length() - 1, 1);
        tokens.push_back(str);
        tokens.push_back("{");
    } else {
        tokens.push_back(str);
    }
}

string Lexer::varTokens(string str, string symbol) {
    std::size_t symbolPos = str.find(symbol);
    tokens.push_back(str.substr(0, symbolPos)); //adding var name
    tokens.push_back(symbol); //symbol
    str.erase(0, symbolPos+symbol.length()); //erasing from string var name and symbol
    return str;
}

string Lexer::spacesInText(string line) {
    string str, strHeader;
    line = line.substr(line.find_first_not_of('\t'), line.length());
    if (line[0] == ' ') {
        line = line.substr(line.find_first_not_of(' '), line.length());
    }
    regex initializeVar("var .*");
    if (regex_match(line, initializeVar)) { //want to save space after "var"
        strHeader = "var ";
        line = line.substr(4, line.length());
    }
    size_t space = line.find(' ');
    size_t parentheses = line.find('\"');
    while ((space != string::npos) && (space < parentheses)) { //erase all spaces before first parentheses
        str.append(line, 0, space);
        line = line.erase(0, space+1);
        space = line.find(' ');
    }
    parentheses = line.find('\"', parentheses +1); //find end of sentence
    str.append(line, 0, parentheses); //add sentence to str
    line = line.erase(0, parentheses);
    space = line.find(' '); //looking for spaces after sentence
    while (space != string::npos) {
        str.append(line, 0, space);
        line = line.erase(0, space+1);
        space = line.find(' ');
    }
    str.append(line);
    return strHeader + str;
}

string Lexer::withoutSpaces(string line) {
    string str, token, strHeader;
    line = line.substr(line.find_first_not_of('\t'), line.length());
    if (line[0] == ' ') {
        line = line.substr(line.find_first_not_of(' '), line.length());
    }
    //want to save space after 'va', 'while', or 'if'
    regex initializeVar("var .*");
    regex whileCommand("while .*");
    regex ifCommand("if .*");
    if (regex_match(line, initializeVar)) {
        strHeader = "var ";
        line = line.substr(4, line.length());
    } else if (regex_match(line, whileCommand)) {
        strHeader = "while ";
        line = line.substr(6, line.length());
    } else if (regex_match(line, ifCommand)) {
        strHeader = "if ";
        line = line.substr(3, line.length());
    }
    std::istringstream tokenStream(line);
    while (std::getline(tokenStream, token, ' '))
    {
        str += token;
    }
    return strHeader + str;
}

