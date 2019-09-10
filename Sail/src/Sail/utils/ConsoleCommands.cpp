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

void ConsoleCommands::addCommand(const std::string& command, std::function<void()> function) {
	m_commands.emplace_back(std::pair(command, function));
	
}

const bool ConsoleCommands::execute() {
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

