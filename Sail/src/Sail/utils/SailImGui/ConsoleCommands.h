#pragma once
#include "SailImGuiWindow.h"
#include "../Regex/Regex.h"
#include "imgui.h"
#include <functional>

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
	static const ImVec4 ERROR_COLOR;
	static const ImVec4 WARNING_COLOR;
	static const ImVec4 LOG_COLOR;
public:
	ConsoleCommands();
	ConsoleCommands(bool showWindow);
	~ConsoleCommands();
	//"Command"					For function without paramater
	void addCommand(const std::string& command, const std::function<std::string(void)>& function, const std::string& identifier = "");
	//"Command <string>"		For single string parameter
	void addCommand(const std::string& command, const std::function<std::string(std::string)>& function, const std::string& identifier = "");
	//"Command <float>"			For single int parameter
	void addCommand(const std::string& command, const std::function<std::string(float)>& function, const std::string& identifier = "");
	//"Command <float> ..."		For List of numbers
	void addCommand(const std::string& command, const std::function<std::string(std::vector<int>)>& function, const std::string& identifier = "");
	void addCommand(const std::string& command, const std::function<std::string(std::vector<float>)>& function, const std::string& identifier = "");

	void removeAllCommandsWithIdentifier(const std::string& identifier);

	//using internal textfield
	const bool execute();
	//using external textfield, storing internal one during the execution ofthe external one, then restores the internal one.
	const bool execute(const std::string& command);

	std::string getTextField();
	void setTextField(const std::string text);

	void addLog(const std::string& log, const ImVec4& color = LOG_COLOR);
	const std::vector<std::pair<std::string, ImVec4>>& getLog();
	const std::vector<std::string>& getCommandLog();

	virtual void renderWindow() override;
	virtual void toggleWindow() override;
	virtual void showWindow(bool show) override;

	static int StaticInputCallback(ImGuiTextEditCallbackData* data);

private:
	template <typename T>
	class Command {
	public:
		Command(const std::string& command, const std::function<T>& func, const std::string& identifier)
			: command(command)
			, func(func)
			, identifier(identifier)
		{}
		std::string command;
		std::function<T> func;
		std::string identifier;
	};

	void init();
	void createHelpCommand();

	template<typename T>
	void addCommandInternal(std::vector<Command<T>>& vec, const std::string& command, const std::function<T>& function, const std::string& identifier);
	
	std::string prune(const std::string& command);
	const std::string parseCommand(const std::string& command);

	const bool voidMatch(const std::string& command);
	const bool intMatch(const std::string& command, const std::string& parsedCommand);
	const bool intArrayMatch(const std::string& command, const std::string& parsedCommand);
	const bool floatMatch(const std::string& command, const std::string& parsedCommand);
	const bool floatArrayMatch(const std::string& command, const std::string& parsedCommand);
	const bool stringMatch(const std::string& command, const std::string& parsedCommand);

	int inputCallback(ImGuiTextEditCallbackData* data);

private:
	std::string m_textField;
	std::vector<std::string> m_commandHistory;
	std::vector<std::pair<std::string, ImVec4>> m_textLog;

	// Command storage
	std::vector<Command<std::string(void)>> m_voidCommands;
	std::vector<Command<std::string(std::string)>> m_stringCommands;
	std::vector<Command<std::string(float)>> m_numberCommands;
	std::vector<Command<std::string(std::vector<int>)>> m_intArrayCommands;
	std::vector<Command<std::string(std::vector<float>)>> m_floatArrayCommands;

	// List of ALL command names <command, identifier>
	std::vector<std::pair<std::string, std::string>> m_commandNames;

	bool m_scrollToBottom;
	bool m_grabKeyboard;
	bool m_enableDisabling;
	int m_historyPos;

	// Animation
	float m_animTime;
	bool m_animRunning;
	
};

template<typename T>
inline void ConsoleCommands::addCommandInternal(std::vector<Command<T>>& vec, const std::string& command, const std::function<T>& function, const std::string& identifier) {
	std::string cmd = prune(command);
	// Add if command doesn't already exist 
	if (std::find_if(m_commandNames.begin(), m_commandNames.end(), [&command](const std::pair<std::string, std::string>& pair) { return pair.first == command; }) == m_commandNames.end()) {
		vec.emplace_back(cmd, function, identifier);
		m_commandNames.emplace_back(cmd, identifier);
	} else {
		Logger::Warning("Tried to register duplicate command to console: " + command);
	}
}
