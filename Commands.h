
#ifndef TASK3_COMMANDS_H
#define TASK3_COMMANDS_H

#include "Expression.h"
#include <queue>
#include "Var.h"
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <stdio.h>
#include <vector>

static bool is_done = false;

/**
 * Command Interface
 */
class Command {

public:
    virtual int execute(vector<string> :: iterator) = 0;
    virtual ~Command() {}
};

class OpenServerCommand : public Command {
    vector<double> *simVals; //values from simulator.
    Interpreter *interpreter;
    mutex* m;
    int client_socket;
    bool notAcceptSim = true;
public:
    OpenServerCommand(vector<double> *simVal, mutex* mut, Interpreter* i) {
        simVals = simVal;
        m = mut;
        interpreter = i;
        is_done = false;
    }
    virtual int execute(vector<string> :: iterator);
    void openServer();
    void insertValuesToVector(char*);
    virtual ~OpenServerCommand() {}
};

class ConnectCommand : public Command {
    queue<string> *simulatorQueue; //string to set in simulator
    Interpreter *interpreter;
    mutex* m;
    int connect_socket;
    bool notConnectToSim = true;
public:
    ConnectCommand(queue<string>*, mutex*, Interpreter*);
    virtual int execute(vector<string> :: iterator);
    void connectToSimulator();
    virtual ~ConnectCommand() {}
};

class DefineVarCommand: public Command {
    map<string, int> posInVector;
    unordered_map<string, Var> *varsToSim;
    unordered_map<string, int> *varsFromSim;
    vector<double> *simVals;
    Interpreter *interpreter;
public:
    DefineVarCommand(unordered_map<string, Var> *varsToSimulator, unordered_map<string, int> *varsFromSimulator,
                     vector<double> *simulatorVals, Interpreter *i);
    virtual int execute(vector<string> :: iterator);
    virtual ~DefineVarCommand() {}
};

class UpdateVarCommand: public Command {
    unordered_map<string, Var> *varsToSim;
    queue<string> *simulatorQueue;
    Interpreter *interpreter;
    mutex *m;
public:
    UpdateVarCommand(unordered_map<string, Var> *varsToSimulator, queue<string> *simQueue,
                     Interpreter *i, mutex* m);
    virtual int execute(vector<string> :: iterator);
    virtual ~UpdateVarCommand() {}
};

class PrintCommand: public Command {
    Interpreter *interpreter;
public:
    PrintCommand(Interpreter *i);
    virtual int execute(vector<string> :: iterator);
    virtual ~PrintCommand() {}
};

class ConditionCommand: public Command {
protected:
    UpdateVarCommand *update;
    unordered_map<string, Command*> *commands;
    Interpreter *interpreter;

    vector<string> conditionArgument(string str);
    vector<string> conditionTokens(const string &str, string condition, int conLen);
    int executeCommands(vector<string> :: iterator);
public:
    bool trueCondition(const vector<string> &conTokens);
    virtual ~ConditionCommand() {}
};

class WhileCommand: public ConditionCommand {
public:
    WhileCommand(UpdateVarCommand *updateVar, unordered_map<string, Command*> *commandsMap, Interpreter *i);
    virtual int execute(vector<string> :: iterator);
    virtual ~WhileCommand() {}
};

class IfCommand: public ConditionCommand {
public:
    IfCommand(UpdateVarCommand *updateVar, unordered_map<string, Command*> *commandsMap, Interpreter *i);
    virtual int execute(vector<string> :: iterator);
    virtual ~IfCommand() {}
};

class SleepCommand : public Command {
    Interpreter *interpreter;
public:
    SleepCommand(Interpreter *i);
    virtual int execute(vector<string> :: iterator);
    virtual ~SleepCommand() {}
};

class CreateFuncCommand : public Command {
    unordered_map<string, Command*> *commands;
public:
    CreateFuncCommand(unordered_map<string, Command*> *);
    virtual int execute(vector<string> :: iterator);
    virtual ~CreateFuncCommand() {}
};

class FuncRunnerCommand : public Command {
    unordered_map<string, Command*> *commands;
    vector<string> :: iterator startFunc;
    string paramName;
    int numStrInFunc;
public:
    FuncRunnerCommand(unordered_map<string, Command*>*, vector<string> :: iterator, string, int);
    virtual int execute(vector<string> :: iterator);
    virtual ~FuncRunnerCommand() {}
};

#endif //TASK3_COMMANDS_H
