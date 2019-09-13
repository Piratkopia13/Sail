#pragma once
#include <functional>
#include "SailImGui/SailImGuiWindow.h"
#include "Regex/Regex.h"


/*
	How to create a new type of input parameters:
	
	1. Create a new addCommand
	2. Create a new vector with desired inputs
	2.5 Create a new case in parseCommand and a new Regex case if an input is not a string float or int
	3. Create a new Match function
	4. add match function to execute();
	5. add vector to help function if you want them visible in "Help"

	(I think that's it)
	Ask Henrik if you have any questions

*/


class ConsoleCommands : public SailImGuiWindow {
public:

	ConsoleCommands();
	ConsoleCommands(const bool windowState);
	~ConsoleCommands();
	//"Command"					For function without paramater
	void addCommand(const std::string& command, const std::function<std::string(void)> function);
	//"Command <string>"		For single string parameter
	void addCommand(const std::string& command, const std::function<std::string(std::string)> function);
	//"Command <float>"			For single int parameter
	void addCommand(const std::string& command, const std::function<std::string(float)> function);
	//"Command <float> ..."		For List of numbers
	void addCommand(const std::string& command, const std::function<std::string(std::vector<int>)> function);
	void addCommand(const std::string& command, const std::function<std::string(std::vector<float>)> function);


	//using internal textfield
	const bool execute();
	//using external textfield, storing internal one during the execution ofthe external one, then restores the internal one.
	const bool execute(const std::string& command);

	std::string getTextField();
	void setTextField(const std::string text);

	const std::vector<std::string>& getLog();
	const std::vector<std::string>& getCommandLog();

private:
	void createHelpCommand();
	
	std::string prune(const std::string& command);
	const std::string parseCommand(const std::string& command);

	const bool voidMatch(const std::string& command);
	const bool intMatch(const std::string& command, const std::string& parsedCommand);
	const bool intArrayMatch(const std::string& command, const std::string& parsedCommand);
	const bool floatMatch(const std::string& command, const std::string& parsedCommand);
	const bool floatArrayMatch(const std::string& command, const std::string& parsedCommand);
	const bool stringMatch(const std::string& command, const std::string& parsedCommand);


	std::string m_textField;
	std::vector<std::string> m_commandHistory;
	std::vector<std::string> m_textLog;

	// command storage
	std::vector<std::pair<std::string, std::function<std::string(void)>>> m_voidCommands;
	std::vector<std::pair<std::string, std::function<std::string(std::string)>>> m_stringCommands;
	std::vector<std::pair<std::string, std::function<std::string(float)>>> m_numberCommands;
	std::vector<std::pair<std::string, std::function<std::string(std::vector<int>)>>> m_intArrayCommands;
	std::vector<std::pair<std::string, std::function<std::string(std::vector<float>)>>> m_floatArrayCommands;
	
};

