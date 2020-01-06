# Flight-Simulator-Interpreter
Ex3 Project:

The main object of the program is interpret the code of a flight program ment to fly an airplane in FlightGrear.

The program needs to get a text file of the program to interpret and the relative path to the file as an argument.

-----------------
DATA STRUCTURES:
-----------------
-Vector<string> for the tokens of the splited txt (the lexer does that).

-Unordered map <string, Var> for variables the update the values in the simulator (with sim ->), and variables with no arrorw. String is variable name, and Var holds information about the variable (value, sim).

-Vector<double> that holds all the values read from the simulator, and always holds the most recent values read.

-Unordered map <sting, int> for variables updated from the simulator. String – name of variable. Int – position of the value of the string in the Vector of the values.
*the way we implemented this map, these variables do NOT hold their value, only the position of this value in the vector. Our reasoning was that this way the vector values are always up-to-date, in O(n) time, and we don’t need to worry about updating the relevant variables (for example – no need to keep track which of the values we receive from the simulator were initialized in the program – encapsulation) and the time to access the value of a variable is still O(1).

-Unordered map<string, Command> of the commands used in the file we’re interpreting. String – command name in the file. Command – command object to execute.

-----------------
IMPLEMENTATION:
-----------------

-LEXER:

The first class object to be called. Reads all the lines in the file, and splits it into individual tokens.
The lexer erases all the spaces between different tokens, identifies functions, erases all spaces in expressions and creates one token per expression.

-PROGRAMRUNNER:

Initializes Command objects to be used and puts them in the map. Parses through the tokens and executes all the commands created in the Lexer.

-COMMAND:

Interface shared by all the commands. Has one method execute - returns how much the parser needs to ‘jump’ to the next command.

-DEFINEVARCOMMAND:
Executes commands that start with ‘var’; adds a new variable to the correct data structure based on the tokens after ‘var’.

-UPDATEVARCOMMAND:
Updates the value of variables in the map of variables to simulator (there is no meaning in updating the value of variables that get their value from the simulator), by adding the value to update into a queue that can be accessed by the ConnectCommand (which updates the value in the simulator).

-PRINTCOMMAND:
Print to terminal. Can be either a sentence (string) or an expression.

-CONDITIONCOMMAND:
Abstract class that deals with executing a list of commands if/while a condition holds. Is inherited by WhileCommand and IfCommand.

-OPENSERVERCOMMAND:
Create a server that reads the values from the simulator. Creates a separate thread that detaches from the main one, and ends when the ProgramRunner updates a global variable.

-CONNECTCOMMAND:
Client of the simulator that updates variables to the simulator. Also runs in separate and detached thread. Reads the values in the queue and updates the simulator.

-CREATEFUNCCOMMAND:
Calculate the details of the function (name, length, start, parameter name), create FuncRunnerCommand and add it to command's map with the name of the func as key. That way when we will get to the activation of the func in the txt we will already have the exact command that will do that, according to the func details.

-FUNCRUNNERCOMMAND:
Run the func in the txt and insert the value of the parameter that been given. 

-EXPRESSION:

Interpreter that interprets a sting and creates an Expression that can calculate the numerical value of the content of the sting based on the values of the variables.

-EXECUTE METHODE:
Get's as parameter iterator to the place in the string vector where the command start. Execute the command and return num of jump to get to the next command.
