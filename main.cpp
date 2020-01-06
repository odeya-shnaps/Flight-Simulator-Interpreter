#include <iostream>
#include <vector>
#include "Lexer.h"
#include "ProgramRunner.h"

using namespace std;

int main(int argc, char* argv[]) {
   if (argc == 1) {

        cout<<"missimg argument in command line"<<endl;
        return 0;
    }

    Lexer lexer;
    vector<string> tokens = lexer.lexingFile(argv[1]); //lexing the txt
    ProgramRunner *progrun = new ProgramRunner();
    progrun->runProgram(tokens); // run the program
    delete progrun;

    return 0;
}