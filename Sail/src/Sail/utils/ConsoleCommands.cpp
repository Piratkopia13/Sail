#include "pch.h"
#include "ConsoleCommands.h"

ConsoleCommands::ConsoleCommands() :
	SailImGuiWindow(false),
	m_textField("")
{

}

ConsoleCommands::ConsoleCommands(const bool windowState) :
SailImGuiWindow(windowState) 
{

}

ConsoleCommands::~ConsoleCommands() {
}

void ConsoleCommands::addCommand(const std::string& command, std::function<void()> function, std::function<std::string()> successfulMessage) {
	m_commands.push_back({ command, function, successfulMessage });
	
}

const bool ConsoleCommands::execute() {
	if (m_textField == "") {
		return false;
	}
	parseCommand(m_textField);

	m_textLog.emplace_back(m_textField);
	m_commandHistory.emplace_back(m_textField);
	m_textField = "";
	return true;
}

const bool ConsoleCommands::execute(const std::string& command) {

	return false;
}

std::string ConsoleCommands::getTextField() {
	return m_textField;
}

void ConsoleCommands::setTextField(const std::string text) {
	if (m_textField != text) {
		int i = 0;
	}
	m_textField = text;
}

const std::vector<std::string>& ConsoleCommands::getLog() {
	return m_textLog;
}

std::string ConsoleCommands::prune(const std::string& command) {
	std::string temp = command.substr(0, command.find_last_not_of(" ") + 1);
	return std::string();
}

const bool ConsoleCommands::voidMatch(const std::string& command) {
	for (int i = 0; i < m_commands.size(); i++) {
		if (m_commands[i].command == command) {
			m_commands[i].function();
			return true;
		}
	}
	return false;
}

const std::string ConsoleCommands::parseCommand(const std::string& command) {
	std::string cmd = prune(command);
	if (voidMatch(cmd)) {
		return "";
	}


	int location = cmd.find_first_of(" ");
	



	int pos = command.find(" ");




	return std::string();
}

