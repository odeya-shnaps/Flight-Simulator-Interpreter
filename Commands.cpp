#include "Commands.h"

using namespace std;

/*
 * creating server and accepting the simulator as a client, and read the data he send.
 */
int OpenServerCommand :: execute(vector<string> :: iterator it) {
    string strPort = *(it+1); //string of the port
    Expression* expression = interpreter->interpret(strPort);
    int port = (int)(expression->calculate());//calculate the port
    delete expression;
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd == -1) {
        std::cerr << "Could not create a socket" << std::endl;
    }

    //bind socket to IP address
    // we first need to create the sockaddr obj.
    sockaddr_in address;
    address.sin_family = AF_INET; //in means IP4
    address.sin_addr.s_addr = INADDR_ANY; //give me any IP allocated for my machine
    address.sin_port = htons(port);

    //the actual bind command
    if (bind(socketfd, (struct sockaddr *) &address, sizeof(address)) == -1) {
        std::cerr << "Could not bind the socket to an IP" << std::endl;
    }

    //making socket listen to the port
    if (listen(socketfd, 5) == -1) { //can also set to SOMAXCON (max connections)
        std::cerr << "Error during listening command" << std::endl;
    }

    while (notAcceptSim) {// loop until client accepted
        // accepting a client
        client_socket = accept(socketfd, (struct sockaddr *) &address,
                               (socklen_t *) &address);
        if (client_socket == -1) {
            std::cerr << "Error accepting client" << std::endl;
        } else {
            notAcceptSim = false;
        }
    }

    close(socketfd); //closing the listening socket

    char data[1000] = {'\0'};
    int readData = read(client_socket, data, 1000); //read data from client
    if (readData == -1) {
        std::cout << "problem in reading from sim" << std::endl;
    } else {
        insertValuesToVector(data);
    }
    // open thread that read data from client and update the values in simVals vector
    thread server(&OpenServerCommand::openServer, this);
    server.detach();
    return 2;
}

/*
 * thread that reads data from simulator and insert it to vector simVals until the program is done.
 */
void OpenServerCommand :: openServer() {

    while(!is_done) {
        char data[1080] = {'\0'};
        int readData = read(client_socket, data, 1080);
        if (readData == -1) {
            std::cout << "problem in reading from sim" << std::endl;
        } else {
            m->lock();
            simVals->clear();
            insertValuesToVector(data);
            m->unlock();
        }
    }

    close(client_socket);
}

/*
 * split the data to tokens of the values and insert to vector simVals.
 */
void OpenServerCommand :: insertValuesToVector(char* data) {

    int i=0;
    char* token;
    token = strtok(data, "\n");//split by '\n'
    char* firstToken = token; //save first token;
    token = strtok(NULL, "\n"); //take the second token where there are all the values.
    if (token == NULL) { // if there is only one token (like in the first time).
        token = firstToken;
    }
    char* value = strtok(token, ","); // split by ','
    while (value != NULL && i < 36) {
        simVals->push_back(stod(value));//insert to vector
        value = strtok(NULL, ",");
        i++;
    }
}

ConnectCommand :: ConnectCommand(queue<string> *simulatorQu, mutex* mut, Interpreter* i) {
    simulatorQueue = simulatorQu;
    m = mut;
    interpreter = i;
}

/*
 * connect to simulator as client and send updates.
 */
int ConnectCommand :: execute(vector<string> :: iterator it) {

    string strPort = *(it+2); //string of the port
    Expression* expression = interpreter->interpret(strPort);
    int port = (int)(expression->calculate()); //port in numbers
    delete expression;
    string str = *(it+1); // ip with ""
    const char* ip = (str.substr(1, str.length()-2)).c_str(); //remove ""
    //create socket
    connect_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (connect_socket == -1) {
        std::cerr << "Could not create a socket" << std::endl;
    }

    // we first need to create the socket's address obj.
    sockaddr_in address;
    address.sin_family = AF_INET; //in means IP4
    address.sin_addr.s_addr = inet_addr(ip); //the local host address
    address.sin_port = htons(port);


    while (notConnectToSim) { // loop until we connect to simulator.
        //connect us to simulator
        int is_connect = connect(connect_socket, (struct sockaddr *) &address, sizeof(address));
        if (is_connect == -1) {
            std::cerr << "Could not connect to host server" << std::endl;
            this_thread::sleep_for(chrono::milliseconds(20));
        } else {
            notConnectToSim = false;
        }
    }
    //open thread that send updates to simulator
    thread connect(&ConnectCommand::connectToSimulator, this);
    connect.detach();
    return 3;
}
/*
 * thread that sending strings of command to the simulator until the program is done.
 */
void ConnectCommand :: connectToSimulator() {

    while (!is_done) {
        m->lock();
        if (!simulatorQueue->empty()) {
            const char* set = (simulatorQueue->front()).c_str(); // convert the string to char*
            int update = send(connect_socket, set, strlen(set), 0); //sending the command to simulator
            if (update == -1) {
                std::cout << "error in sending" << std::endl;
            } else {
                simulatorQueue->pop(); //remove the command that we sent
            }
        }
        m->unlock();
    }

    close(connect_socket);
}

DefineVarCommand:: DefineVarCommand(unordered_map<string, Var> *varsToSimulator,
                                    unordered_map<string, int> *varsFromSimulator, vector<double> *simulatorVals, Interpreter *i) {
    varsToSim = varsToSimulator;
    varsFromSim = varsFromSimulator;
    simVals = simulatorVals;
    interpreter = i;
    //map of sim values to position in vactor. used to fined vector position of variables based on the sim
    posInVector.insert({"/instrumentation/airspeed-indicator/indicated-speed-kt", 0});
    posInVector.insert({"/sim/time/warp", 1});
    posInVector.insert({"/controls/switches/magnetos", 2});
    posInVector.insert({"/instrumentation/heading-indicator/offset-deg", 3});
    posInVector.insert({"/instrumentation/altimeter/indicated-altitude-ft", 4});
    posInVector.insert({"/instrumentation/altimeter/pressure-alt-ft", 5});
    posInVector.insert({"/instrumentation/attitude-indicator/indicated-pitch-deg", 6});
    posInVector.insert({"/instrumentation/attitude-indicator/indicated-roll-deg", 7});
    posInVector.insert({"/instrumentation/attitude-indicator/internal-pitch-deg", 8});
    posInVector.insert({"/instrumentation/attitude-indicator/internal-roll-deg", 9});
    posInVector.insert({"/instrumentation/encoder/indicated-altitude-ft", 10});
    posInVector.insert({"/instrumentation/encoder/pressure-alt-ft", 11});
    posInVector.insert({"/instrumentation/gps/indicated-altitude-ft", 12});
    posInVector.insert({"/instrumentation/gps/indicated-ground-speed-kt", 13});
    posInVector.insert({"/instrumentation/gps/indicated-vertical-speed", 14});
    posInVector.insert({"/instrumentation/heading-indicator/indicated-heading-deg", 15});
    posInVector.insert({"/instrumentation/magnetic-compass/indicated-heading-deg", 16});
    posInVector.insert({"/instrumentation/slip-skid-ball/indicated-slip-skid", 17});
    posInVector.insert({"/instrumentation/turn-indicator/indicated-turn-rate", 18});
    posInVector.insert({"/instrumentation/vertical-speed-indicator/indicated-speed-fpm", 19});
    posInVector.insert({"/controls/flight/aileron", 20});
    posInVector.insert({"/controls/flight/elevator", 21});
    posInVector.insert({"/controls/flight/rudder", 22});
    posInVector.insert({"/controls/flight/flaps", 23});
    posInVector.insert({"/controls/engines/engine/throttle", 24});
    posInVector.insert({"/controls/engines/current-engine/throttle", 25});
    posInVector.insert({"/controls/switches/master-avionics", 26});
    posInVector.insert({"/controls/switches/starter", 27});
    posInVector.insert({"/engines/active-engine/auto-start", 28});
    posInVector.insert({"/controls/flight/speedbrake", 29});
    posInVector.insert({"/sim/model/c172p/brake-parking", 30});
    posInVector.insert({"/controls/engines/engine/primer", 31});
    posInVector.insert({"/controls/engines/current-engine/mixture", 32});
    posInVector.insert({"/controls/switches/master-bat", 33});
    posInVector.insert({"/controls/switches/master-alt", 34});
    posInVector.insert({"/engines/engine/rpm", 35});
}

int DefineVarCommand:: execute(vector<string> :: iterator itr) {
    int counter = 5;
    string varName = *(itr+1); //second token is var name
    string symbol = *(itr+2); //third token is symbol
    if (symbol == "->") {
        string sim = (*(itr+4)).substr(1, (*(itr+4)).length()-2); //fifth position is sim string
        Var var(sim);
        varsToSim->insert(pair<string, Var>(varName, var));
    } else if (symbol == "=") {
        Expression* expression = interpreter->interpret((*(itr+3)));
        double value = expression->calculate();
        delete expression;
        Var var(value);
        varsToSim->insert(pair<string, Var>(varName, var));
        counter = 4; //if was '=' so only 4 tokens were used
    } else {
        string sim = (*(itr+4)).substr(1, (*(itr+4)).length()-2);
        int pos = posInVector[sim];
        varsFromSim->insert({varName, pos});
    }
    return counter;
}

UpdateVarCommand:: UpdateVarCommand (unordered_map<string, Var> *varsToSimulator, queue<string> *simQueue,
                                     Interpreter *i, mutex* mut) {
    varsToSim = varsToSimulator;
    simulatorQueue = simQueue;
    interpreter = i;
    m = mut;
}

int UpdateVarCommand:: execute(vector<string>::iterator itr) {
    string varName = *itr;
    //find new value of variable
    Expression* expression = interpreter->interpret((*(itr+2)));
    double value = expression->calculate();
    delete expression;
    //if variable in varToSim map - update variable in map and add to queue
    unordered_map<string, Var>::iterator it = varsToSim->find(varName);
    if (it != varsToSim->end()) {
        it->second.setValue(value);
        string sim = it->second.getSim();
        string str = "set "+sim+" "+to_string(value)+"\r\n"; //string to send to simulator
        m->lock();
        simulatorQueue->push(str); //queue of values to uodate in the simulator
        m->unlock();
    }
    return 3;
}

PrintCommand::PrintCommand(Interpreter *i) {
    interpreter = i;
}

int PrintCommand::execute(vector<string>::iterator itr) {
    string toPrint = *(itr+1);
    if (toPrint[0] == '\"') { //need to print a string
        cout<<toPrint.substr(1, toPrint.length()-2)<<endl;
    } else { //need to calculate expression
        Expression* expression = interpreter->interpret(toPrint);
        cout<<expression->calculate()<<endl;
        delete expression;
    }
    return 2;
}

WhileCommand::WhileCommand(UpdateVarCommand *updateVar, unordered_map<string, Command*> *commandsMap, Interpreter *i) {
    update = updateVar;
    commands = commandsMap;
    interpreter = i;
}

int WhileCommand:: execute(vector<string>::iterator itr) {
    int numOfTokens = 0;
    vector<string> conTokens = conditionArgument(*(itr+1)); //split while condition to it's parts
    itr+=3; //go past tokens of: while, condition, {
    if (!trueCondition(conTokens)) {//condition is false. do not need to perform loop even once
        for (auto it = itr; *it != "}"; it++) { //find end of loop to return correct int
            numOfTokens ++;
        }
        return numOfTokens + 4; //jump past first 3 + all commands in loop + last }
    }
    while (trueCondition(conTokens)) { //while condition is true execute all commands in loop
        numOfTokens = executeCommands(itr);
    }
    return numOfTokens + 4;
}

/*
 * check if the condition is true
 */
bool ConditionCommand::trueCondition(const vector<string> &conTokens) {
    if (conTokens.size() == 1) { //condition has 1 token. return it
        Expression *e = interpreter->interpret(conTokens[0]);
        double num = e->calculate();
        delete e;
        return num;
    } else { //condition has 3 parrts
        string left = conTokens[0], symbol = conTokens[1], right = conTokens[2];
        Expression *e1 = interpreter->interpret(left);
        Expression *e2 = interpreter->interpret(right);
        double num1, num2;
        num1 = e1->calculate();
        num2 = e2->calculate();
        delete e1;
        delete e2;
        if (symbol == "<") {
            return (num1 < num2);
        } else if (symbol == ">") {
            return (num1 > num2);
        } else if (symbol == "<=") {
            return (num1 <= num2);
        } else if (symbol == ">=") {
            return (num1 >= num2);
        } else if (symbol == "==") {
            return (num1 == num2);
        } else {
            return (num1 != num2);
        }
    }
}

/*
 * execute all the comands in the scope of the condition
 */
int ConditionCommand:: executeCommands(vector<string> :: iterator itr) {
    /*
    * jump says how much the iterator needs to move forward after each command.
    * counter counts how many tokens all together to return at the end of execute
    */
    int counter = 0, jump;
    Command *c;
    while (*itr != "}") {
        c = (*commands)[*itr];
        if (c != nullptr) {
            jump = c->execute(itr);
        } else {
            commands->erase(*itr); //so variable names won't be in program map
            jump = update->execute(itr);
        }
        itr+= jump;
        counter+= jump;
    }
    itr -= counter;
    return counter;
}

/*
 * split string in symbol pos and return vector
 */
vector<string> ConditionCommand:: conditionTokens(const string &str, string condition, int conLen) {
    vector<string> conTokens;
    size_t conPos = str.find(condition);
    conTokens.push_back(str.substr(0, conPos));
    conTokens.push_back(str.substr(conPos, conLen));
    conTokens.push_back(str.substr(conPos+conLen));
    return conTokens;
}

/*
 * find symbol in condition token, split the condition string to it's parts, and return as vector
 */
vector<string> ConditionCommand:: conditionArgument(string str) {
    if (str.find("<=") != string::npos) {
        return conditionTokens(str, "<=", 2);
    } else if (str.find(">=") != string::npos) {
        return conditionTokens(str, ">=", 2);
    } else if (str.find("==") != string::npos) {
        return conditionTokens(str, "==", 2);
    } else if (str.find("!=") != string::npos) {
        return conditionTokens(str, "!=", 2);
    } else if (str.find(">") != string::npos) {
        return conditionTokens(str, ">", 1);
    } else if (str.find("<") != string::npos) {
        return conditionTokens(str, "<", 1);
    } else { //condition is 1 word
        vector<string> conTokens;
        conTokens.push_back(str);
        return conTokens;
    }
}

IfCommand::IfCommand(UpdateVarCommand *updateVar, unordered_map<string, Command*> *commandsMap, Interpreter *i) {
    update = updateVar;
    commands = commandsMap;
    interpreter = i;
}

int IfCommand::execute(vector<string>::iterator itr) {
    int numOfTokens = 0;
    vector<string> conTokens = conditionArgument(*(itr+1)); //split if condition to it's parts
    itr+=3; //go past tokens of: if, condition, {
    if (!trueCondition(conTokens)) { //condition is false. do not need to perform commands in scope
        for (auto it = itr; *it != "}"; it++) { //find end of loop to return corrct int
            numOfTokens ++;
        }
        return numOfTokens + 4; //jump past first 3 + all commands in if + last }
    }
    else { //condition is true - execute all commands
        numOfTokens = executeCommands(itr);
    }
    return numOfTokens + 4;
}

SleepCommand :: SleepCommand(Interpreter *i) {
    interpreter = i;
}

/*
 * let the thread that we are at sleep.
 */
int SleepCommand ::execute(vector<string>::iterator it) {
    string millisec = *(it+1);// string of the time to sleep
    Expression* expression = interpreter->interpret(millisec);
    int milisec = expression->calculate(); //calculate the time to numbers
    this_thread::sleep_for(chrono::milliseconds(milisec)); //let this thread sleep
    delete expression;
    return 2;
}

CreateFuncCommand :: CreateFuncCommand(unordered_map<string, Command*> *commandsMap) {
    commands = commandsMap;
}

/*
 * Create FuncRunnerCommand according to the func details and add it to the command's map.
 */
int CreateFuncCommand ::execute(vector<string>::iterator it) {

    vector<string>::iterator startFunc = it + 2; //start of the func '{'
    string nameFunc = *it;
    string parameterName = (*(it + 1)).substr(3, (*(it + 1)).length() - 3);
    int numStrInFunc = 1;
    it = it + 3; //skip name of func,parameter name and first {.
    //count the num of strings in the func from { to }
    int count = 1; //num that check that we arrive to the '}' of the func (and not of the loop or if)
    while (count != 0) {
        if (*it == "{") {
            count++;
        }
        if (*it == "}") {
            count--;
        }
        numStrInFunc++;
        it++;
    }
    //create the func command with the right details.
    FuncRunnerCommand *func = new FuncRunnerCommand(commands, startFunc, parameterName, numStrInFunc);
    (*commands)[nameFunc] = func;
    return numStrInFunc + 2;
}

FuncRunnerCommand ::FuncRunnerCommand(unordered_map<string, Command *> *commandsMap, vector<string>::iterator theStart,
        string parameterName, int numStr) {
    commands = commandsMap;
    startFunc = theStart;
    paramName = parameterName;
    numStrInFunc = numStr;
}

/*
 * Insert parameter value and run the func.
 */
int FuncRunnerCommand ::execute(vector<string>::iterator it) {
    string valueOfParam = *(it+1);// string of the value of the parameter.
    //vector parameter details to send to DefineVarCommand.
    vector<string> paramDetail = {"var", paramName, "=", *(it + 1)};
    auto itToDetails = paramDetail.begin();
    Command *insertParam = (*commands)["var"]; //find DefineVarCommand in the map.
    insertParam->execute(itToDetails); //create the param and insert to the right map.

    int i = 1; //because we count '{'
    startFunc++;
    Command *c;
    // run the func until get to '}'
    while (i < numStrInFunc) {
        c = (*commands)[*startFunc];
        if (c == nullptr) {
            //code of func can't be inside another code of func (only the call to another func)
            // so there is no need to check if needed create func command.
            c = (*commands)["Update"];
        }
        int jump = c->execute(startFunc);
        startFunc += jump;
        i += jump;
    }

    return 2;
}