#pragma once
#include <functional>
#include "SailImGui/SailImGuiWindow.h"
#include "Regex/Regex.h"

class ConsoleCommands : public SailImGuiWindow {
public:

	ConsoleCommands();
	ConsoleCommands(const bool windowState);
	~ConsoleCommands();

	void addCommand(const std::string& command, std::function<void()> function, std::function<std::string()> successfulMessage);
	//using internal textfield
	const bool execute();
	//using external textfield
	const bool execute(const std::string& command);
	std::string getTextField();
	void setTextField(const std::string text);

	const std::vector<std::string>& getLog();

private:
	struct Command {
		std::string command;
		std::function<void()> function;
		std::function<std::string()> successfulMessage;
	};
	std::string prune(const std::string& command);
	const bool voidMatch(const std::string& command);
	const std::string parseCommand(const std::string& command);

	std::string m_textField;
	std::vector<std::string> m_commandHistory;
	std::vector<std::string> m_textLog;
	std::vector<Command> m_commands;
};

