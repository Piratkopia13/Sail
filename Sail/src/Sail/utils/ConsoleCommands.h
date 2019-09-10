#pragma once
#include <functional>
#include "SailImGui/SailImGuiWindow.h"

class ConsoleCommands : public SailImGuiWindow {
public:
	ConsoleCommands();
	ConsoleCommands(const bool windowState);
	~ConsoleCommands();

	void addCommand(const std::string& command, std::function<void()> function);
	//using internal textfield
	const bool execute();
	//using external textfield
	const bool execute(const std::string& command);
	std::string getTextField();
	void setTextField(const std::string text);

	const std::vector<std::string>& getLog();

private:
	const bool parseCommand(const std::string& command);


	std::string m_textField;
	std::vector<std::string> m_commandHistory;
	std::vector<std::string> m_textLog;
	std::vector<std::pair<std::string, std::function<void()>>> m_commands;
};

