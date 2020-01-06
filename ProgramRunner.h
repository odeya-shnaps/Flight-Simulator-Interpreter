
#ifndef TASK3_PROGRAMRUNNER_H
#define TASK3_PROGRAMRUNNER_H

#include <string>
#include "Commands.h"

class ProgramRunner {
    //data structures
    unordered_map<string, Var> varsToSim; //vars that update simulator
    unordered_map<string, int> varsFromSim; //vars that get their value from simulator
    unordered_map<string, Command*> progCommands; //map of all the commands
    vector<double> simVals; //vector with all the values from simulator (update in open server thread).
    queue<string> simulatorQueue; //string of commands to simulator (sent in connect client thread).
    mutex mOpenServer; //mutex to lock the code while we update the vector simVals.
    mutex mConnect; //mutex to lock the code while we sent string to simulator and delete it from simulatorQueue.
    Interpreter *i; //create expression from string
public:
    ProgramRunner();
    void runProgram(vector<string> tokens);
    ~ProgramRunner();
};

#endif //TASK3_PROGRAMRUNNER_H
