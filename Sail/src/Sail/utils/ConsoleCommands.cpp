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

void ConsoleCommands::addCommand(const std::string& command, const std::function<std::string(void)> function) {
	m_voidCommands.push_back({ command, function});
}
void ConsoleCommands::addCommand(const std::string& command, const std::function<std::string(std::string)> function) {
	m_stringCommands.push_back({ command, function});

}
void ConsoleCommands::addCommand(const std::string& command, const std::function<std::string(float)> function) {
	m_numberCommands.push_back({ command, function });
}
void ConsoleCommands::addCommand(const std::string& command, const std::function<std::string(std::vector<int>)> function) {
	m_intArrayCommands.push_back({ command, function });
}
void ConsoleCommands::addCommand(const std::string& command, const std::function<std::string(std::vector<float>)> function) {
	m_floatArrayCommands.push_back({ command, function });
}

const bool ConsoleCommands::execute() {
	if (m_textField == "") {
		return false;
	}
	std::string cmd = prune(m_textField);
	if (voidMatch(cmd)) {
		return true;
	}
	std::string parsedCommand = parseCommand(cmd);
	if (intMatch(m_textField, parsedCommand)) {
		m_textField = "";
		return true;
	}
	if (floatMatch(m_textField, parsedCommand)) {
		m_textField = "";
		return true;
	}
	if (stringMatch(m_textField, parsedCommand)) {
		m_textField = "";
		return true;
	}
	if (intArrayMatch(m_textField, parsedCommand)) {
		m_textField = "";
		return true;
	}
	if (floatArrayMatch(m_textField, parsedCommand)) {
		m_textField = "";
		return true;
	}

	m_textLog.emplace_back("unknown command: "+m_textField);
	m_textField = "";
	return true;
}
const bool ConsoleCommands::execute(const std::string& command) {
	std::string temp = m_textField;
	m_textField = command;
	bool returnValue = execute();
	m_textField = temp;
	return returnValue;
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
const std::vector<std::string>& ConsoleCommands::getCommandLog() {
	return m_commandHistory;
}

std::string ConsoleCommands::prune(const std::string& command) {
	std::string temp = command.substr(0, command.find_last_not_of(" ") + 1);
	return temp;
}
const std::string ConsoleCommands::parseCommand(const std::string& command) {
	int location = command.find_first_of(" ");
	if (location == std::string::npos)
		return command;
	std::string parsedCommand = command.substr(0, location);
	while (location != std::string::npos) {
		int nextLocation = command.find(" ", location+1);
		std::string section = command.substr(location+1, nextLocation - location -1);
		if (Reg::TextCharacterStar.match(section.c_str()) == section.size()) {
			parsedCommand += " <string>";
		} 
		else if (Reg::Number.match(section.c_str()) == section.size()) {
			parsedCommand += " <int>";
		}
		else if (Reg::DecimalNumber.match(section.c_str()) == section.size()) {
			parsedCommand += " <float>";
		}
		location = nextLocation;
	}



	return parsedCommand;
}

const bool ConsoleCommands::voidMatch(const std::string& command) {
	for (int i = 0; i < m_voidCommands.size(); i++) {
		if (m_voidCommands[i].first == command) {
			m_textLog.emplace_back(m_textField);
			m_commandHistory.emplace_back(command);
			m_textLog.emplace_back(m_voidCommands[i].second());
			m_textField = "";
			return true;
		}
	}
	return false;
}
const bool ConsoleCommands::intMatch(const std::string& command, const std::string& parsedCommand) {
	for (int i = 0; i < m_numberCommands.size(); i++) {
		if (m_numberCommands[i].first == parsedCommand) {
			int location = command.find(" ");
			int size = command.find(" ", location + 1) - location - 1;
			int value = std::stoi(command.substr(location + 1, size));
			
			m_textLog.emplace_back(m_textField);
			m_commandHistory.emplace_back(command);
			m_textLog.emplace_back(m_numberCommands[i].second(value));
			m_textField = "";
			return true;
		}
	}
	return false;
}
const bool ConsoleCommands::intArrayMatch(const std::string& command, const std::string& parsedCommand) {
	for (int i = 0; i < m_intArrayCommands.size(); i++) {
		if (m_intArrayCommands[i].first == parsedCommand) {
			std::vector<int> arr;
			int location = command.find(" ");
			while (location != std::string::npos) {
				int nextLocation = command.find(" ", location + 1);
				std::string section = command.substr(location + 1, nextLocation - location - 1);
				int value = std::stoi(section);
				arr.emplace_back(value);
				location = nextLocation;
			}
			
			m_textLog.emplace_back(m_textField);
			m_commandHistory.emplace_back(command);
			m_textLog.emplace_back(m_intArrayCommands[i].second(arr));
			m_textField = "";
			return true;
		}
	}
	return false;
}
const bool ConsoleCommands::floatMatch(const std::string& command, const std::string& parsedCommand) {
	for (int i = 0; i < m_numberCommands.size(); i++) {
		if (m_numberCommands[i].first == parsedCommand) {
			int location = command.find(" ");
			int size = command.find(" ", location + 1) - location - 1;
			float value = std::stof(command.substr(location + 1, size));

			m_textLog.emplace_back(m_textField);
			m_commandHistory.emplace_back(command);
			m_textLog.emplace_back(m_numberCommands[i].second(value));
			m_textField = "";
			return true;
		}
	}
	return false;
}
const bool ConsoleCommands::floatArrayMatch(const std::string& command, const std::string& parsedCommand) {
	for (int i = 0; i < m_floatArrayCommands.size(); i++) {
		if (m_floatArrayCommands[i].first == parsedCommand) {
			std::vector<float> arr;
			int location = command.find(" ");
			while (location != std::string::npos) {
				int nextLocation = command.find(" ", location + 1);
				std::string section = command.substr(location + 1, nextLocation - location - 1);
				float value = std::stof(section);
				arr.emplace_back(value);
				location = nextLocation;
			}

			m_textLog.emplace_back(m_textField);
			m_commandHistory.emplace_back(command);
			m_textLog.emplace_back(m_floatArrayCommands[i].second(arr));
			m_textField = "";
			return true;
		}
	}
	return false;
}
const bool ConsoleCommands::stringMatch(const std::string& command, const std::string& parsedCommand) {
	for (int i = 0; i < m_stringCommands.size(); i++) {
		if (m_stringCommands[i].first == parsedCommand) {
			int location = command.find(" ");
			int size = command.find(" ", location + 1) - location - 1;
			std::string value = command.substr(location + 1, size);

			m_textLog.emplace_back(m_textField);
			m_commandHistory.emplace_back(command);
			m_textLog.emplace_back(m_stringCommands[i].second(value));
			m_textField = "";
			return true;
		}
	}
	return false;
}


