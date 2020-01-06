#include "ProgramRunner.h"

using namespace std;

ProgramRunner::ProgramRunner() {

    i = new Interpreter(&varsToSim, &varsFromSim, &simVals);
    i->setM(&mOpenServer);

    //create commands and add to map
    OpenServerCommand *openServer = new OpenServerCommand(&simVals, &mOpenServer, i);
    ConnectCommand *connectSim = new ConnectCommand(&simulatorQueue, &mConnect, i);
    DefineVarCommand *defVar = new DefineVarCommand(&varsToSim, &varsFromSim, &simVals, i);
    UpdateVarCommand *update = new UpdateVarCommand(&varsToSim, &simulatorQueue, i, &mConnect);
    PrintCommand *print = new PrintCommand(i);
    WhileCommand *whilecom = new WhileCommand(update, &progCommands, i);
    IfCommand *ifCom = new IfCommand(update, &progCommands, i);
    SleepCommand *sleep = new SleepCommand(i);
    CreateFuncCommand *createFunc = new CreateFuncCommand(&progCommands);
    progCommands["var"] = defVar;
    progCommands["Print"] = print;
    progCommands["Sleep"] = sleep;
    progCommands["Update"] = update;
    progCommands["openDataServer"] = openServer;
    progCommands["connectControlClient"] = connectSim;
    progCommands["while"] = whilecom;
    progCommands["if"] = ifCom;
    progCommands["CreateFunc"] = createFunc;
}

void ProgramRunner:: runProgram(vector<string> tokens) {

    //parser
    vector<string>::iterator itr = tokens.begin();
    Command *c;
    while (itr != tokens.end()) {
        c = progCommands[*itr];
        if (c == nullptr) {
            auto it = varsToSim.find(*itr); //check if a var name to update (we update only varToSim)
            if (it != varsToSim.end()) { //is a var name to update
                progCommands.erase(*itr); //so variable names won't be in program map
                c = progCommands["Update"];
            } else { // name of func that we didn't create.
                c = progCommands["CreateFunc"];
            }
        }
        itr += c->execute(itr);
    }

    is_done = true;
}

ProgramRunner::~ProgramRunner() {

    for (auto it = progCommands.begin(); it != progCommands.end(); it++) {
        delete it->second;
    }
    delete i;
}



