#include "pch.h"
#include "ConsoleCommands.h"
#include "Sail/Application.h"
#include "Sail/KeyBinds.h"

ConsoleCommands::ConsoleCommands()
	: SailImGuiWindow(false)
	, m_textField("")
	, m_scrollToBottom(true)
	, m_grabKeyboard(false)
{
	init();
}
ConsoleCommands::ConsoleCommands(bool showWindow)
	: SailImGuiWindow(showWindow) 
	, m_scrollToBottom(true)
	, m_grabKeyboard(false)
{
	init();
}
ConsoleCommands::~ConsoleCommands() {}

void ConsoleCommands::init() {
	m_historyPos = -1;
	createHelpCommand();
#ifdef _DEBUG
#pragma region TESTCASES
	addCommand(std::string("Save"), []() { return std::string("saved"); });
	addCommand(std::string("Test <int>"), [](int in) { return std::string("test<int>"); });
	addCommand(std::string("Test <float>"), [](float in) { return std::string("test<float>"); });
	addCommand(std::string("Test <string>"), [](std::string in) { return std::string("test<string>"); });
	addCommand(std::string("Test <int> <int> <int>"/*...*/), [](std::vector<int> in) {return std::string("test<std::vector<int>"); });
	addCommand(std::string("Test <float> <float> <float>"/*...*/), [](std::vector<float> in) {return std::string("test<std::vector<float>"); });
#pragma endregion
#endif
}

void ConsoleCommands::addCommand(const std::string& command, const std::function<std::string(void)>& function, const std::string& identifier) {
	addCommandInternal(m_voidCommands, command, function, identifier);
}
void ConsoleCommands::addCommand(const std::string& command, const std::function<std::string(std::string)>& function, const std::string& identifier) {
	addCommandInternal(m_stringCommands, command, function, identifier);
}
void ConsoleCommands::addCommand(const std::string& command, const std::function<std::string(float)>& function, const std::string& identifier) {
	addCommandInternal(m_numberCommands, command, function, identifier);
}
void ConsoleCommands::addCommand(const std::string& command, const std::function<std::string(std::vector<int>)>& function, const std::string& identifier) {
	addCommandInternal(m_intArrayCommands, command, function, identifier);
}
void ConsoleCommands::addCommand(const std::string& command, const std::function<std::string(std::vector<float>)>& function, const std::string& identifier) {
	addCommandInternal(m_floatArrayCommands, command, function, identifier);
}

void ConsoleCommands::removeAllCommandsWithIdentifier(const std::string& identifier) {
	// Remove from commandNames
	m_commandNames.erase(std::remove_if(m_commandNames.begin(), m_commandNames.end(), [&identifier](const std::pair<std::string, std::string>& cmd) {
		return cmd.second == identifier; }), m_commandNames.end());

	// Remove from void commands
	m_voidCommands.erase(std::remove_if(m_voidCommands.begin(), m_voidCommands.end(), [&identifier](const ConsoleCommands::Command<std::string(void)>& cmd) {
		return cmd.identifier == identifier; }), m_voidCommands.end());
	// Remove from string commands
	m_stringCommands.erase(std::remove_if(m_stringCommands.begin(), m_stringCommands.end(), [&identifier](const ConsoleCommands::Command<std::string(std::string)>& cmd) {
		return cmd.identifier == identifier; }), m_stringCommands.end());
	// Remove from number commands
	m_numberCommands.erase(std::remove_if(m_numberCommands.begin(), m_numberCommands.end(), [&identifier](const ConsoleCommands::Command<std::string(float)>& cmd) {
		return cmd.identifier == identifier; }), m_numberCommands.end());
	// Remove from int array commands
	m_intArrayCommands.erase(std::remove_if(m_intArrayCommands.begin(), m_intArrayCommands.end(), [&identifier](const ConsoleCommands::Command<std::string(std::vector<int>)>& cmd) {
		return cmd.identifier == identifier; }), m_intArrayCommands.end());
	// Remove from float array commands
	m_floatArrayCommands.erase(std::remove_if(m_floatArrayCommands.begin(), m_floatArrayCommands.end(), [&identifier](const ConsoleCommands::Command<std::string(std::vector<float>)>& cmd) {
		return cmd.identifier == identifier; }), m_floatArrayCommands.end());
}

const bool ConsoleCommands::execute() {
	if (m_textField == "") {
		return false;
	}
	// Add to history if new
	if (m_commandHistory.empty() || m_commandHistory.back() != m_textField) {
		m_commandHistory.emplace_back(m_textField);
	}
	m_historyPos = -1;

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

	addLog("Unknown command: " + m_textField);
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

void ConsoleCommands::addLog(const std::string& log) {
	m_textLog.emplace_back(log);
	m_scrollToBottom = true;
}

const std::vector<std::string>& ConsoleCommands::getLog() {
	return m_textLog;
}
const std::vector<std::string>& ConsoleCommands::getCommandLog() {
	return m_commandHistory;
}

void ConsoleCommands::renderWindow() {
	if (Input::WasKeyJustPressed(KeyBinds::toggleConsole) || ImGui::IsKeyPressed(KeyBinds::toggleConsole)) {
		// Toggle console
		toggleWindow();
	}

	bool open = isWindowOpen();
	if (open) {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
		static char buf[256] = "";
		if (ImGui::Begin("Console", &open, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove)) {
			auto* window = Application::getInstance()->getWindow();
			ImGui::SetWindowSize(ImVec2((float)window->getWindowWidth(), glm::min((float)window->getWindowHeight(), 200.f)), ImGuiCond_Always);
			ImGui::SetWindowPos(ImVec2(0.f, 0.f), ImGuiCond_Always);

			ImGui::BeginChild("ScrollingRegion", ImVec2(0, -23), false, ImGuiWindowFlags_HorizontalScrollbar);

			for (int i = 0; i < getLog().size(); i++) {
				ImGui::TextUnformatted(getLog()[i].c_str());
			}

			if (m_scrollToBottom) {
				// Scroll to bottom
				ImGui::SetScrollHere(1.f);
				m_scrollToBottom = false;
			}

			ImGui::EndChild();
			ImGui::Separator();

			getTextField().copy(buf, getTextField().size() + 1);
			buf[getTextField().size()] = '\0';

			std::string original = getTextField();
			if (m_grabKeyboard) {
				ImGui::SetKeyboardFocusHere();
				m_grabKeyboard = false;
			}

			ImGui::SetNextItemWidth((float)window->getWindowWidth() - 85.f);
			bool exec = ImGui::InputText("", buf, IM_ARRAYSIZE(buf), 
				ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_CallbackCompletion, &StaticInputCallback, (void*)this);
			ImGui::SameLine();
			if (exec || ImGui::Button("Execute", ImVec2(0, 0))) {
				execute(); // TODO: do something with return value(?)
				m_grabKeyboard = true;
			} else {
				setTextField(std::string(buf));
			}
		}
		ImGui::End();
		ImGui::PopStyleVar(1);
	}
	m_enableDisabling = true;
}

int ConsoleCommands::StaticInputCallback(ImGuiTextEditCallbackData* data) {
	auto* instance = (ConsoleCommands*)data->UserData;
	return instance->inputCallback(data);
}

void ConsoleCommands::toggleWindow() {
	SailImGuiWindow::toggleWindow();
	if (isWindowOpen()) {
		m_grabKeyboard = true;
	} else {
		m_enableDisabling = false;
	}
}

void ConsoleCommands::showWindow(bool show) {
	SailImGuiWindow::showWindow(show);
	if (isWindowOpen()) {
		m_grabKeyboard = true;
	} else {
		m_enableDisabling = false;
	}
}

void ConsoleCommands::createHelpCommand() {
	addCommand(std::string("Help"), [&]() {
		addLog("");
		addLog("Current Commands:");
		for (int i = 0; i < m_voidCommands.size(); i++) {
			addLog(m_voidCommands[i].command);
		}
		for (int i = 0; i < m_stringCommands.size(); i++) {
			addLog(m_stringCommands[i].command);
		}
		for (int i = 0; i < m_numberCommands.size(); i++) {
			addLog(m_numberCommands[i].command);
		}
		for (int i = 0; i < m_intArrayCommands.size(); i++) {
			addLog(m_intArrayCommands[i].command);
		}
		for (int i = 0; i < m_floatArrayCommands.size(); i++) {
			addLog(m_floatArrayCommands[i].command);
		}

		return std::string("");
	});
}

std::string ConsoleCommands::prune(const std::string& command) {
	std::string temp = command.substr(0, command.find_last_not_of(" ") + 1);
	std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
	return temp;
}
const std::string ConsoleCommands::parseCommand(const std::string& command) {
	size_t location = command.find_first_of(" ");
	if (location == std::string::npos)
		return command;
	std::string parsedCommand = command.substr(0, location);
	while (location != std::string::npos) {
		size_t nextLocation = command.find(" ", location+1);
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
		if (m_voidCommands[i].command == command) {
			addLog(m_textField);
			addLog(m_voidCommands[i].func());
			m_textField = "";
			return true;
		}
	}
	return false;
}
const bool ConsoleCommands::intMatch(const std::string& command, const std::string& parsedCommand) {
	for (int i = 0; i < m_numberCommands.size(); i++) {
		if (m_numberCommands[i].command == parsedCommand) {
			size_t location = command.find(" ");
			size_t size = command.find(" ", location + 1) - location - 1;
			std::string section = command.substr(location + 1, size);
			if (section.size() > 9)
				section = section.substr(0, 9);
			int value = std::stoi(section);
			
			addLog(m_textField);
			addLog(m_numberCommands[i].func(static_cast<float>(value)));
			m_textField = "";
			return true;
		}
	}
	return false;
}
const bool ConsoleCommands::intArrayMatch(const std::string& command, const std::string& parsedCommand) {
	for (int i = 0; i < m_intArrayCommands.size(); i++) {
		if (m_intArrayCommands[i].command == parsedCommand) {
			std::vector<int> arr;
			size_t location = command.find(" ");
			while (location != std::string::npos) {
				size_t nextLocation = command.find(" ", location + 1);
				std::string section = command.substr(location + 1, nextLocation - location - 1);
				if (section.size() > 9)
					section = section.substr(0, 9);
				int value = std::stoi(section);
				arr.emplace_back(value);
				location = nextLocation;
			}
			
			addLog(m_textField);
			addLog(m_intArrayCommands[i].func(arr));
			m_textField = "";
			return true;
		}
	}
	return false;
}
const bool ConsoleCommands::floatMatch(const std::string& command, const std::string& parsedCommand) {
	for (int i = 0; i < m_numberCommands.size(); i++) {
		if (m_numberCommands[i].command == parsedCommand) {
			size_t location = command.find(" ");
			size_t size = command.find(" ", location + 1) - location - 1;

			std::string section = command.substr(location + 1, size);
			if (section.size() > 9)
				section = section.substr(0, 9);
			float value = std::stof(section);

			addLog(m_textField);
			addLog(m_numberCommands[i].func(value));
			m_textField = "";
			return true;
		}
	}
	return false;
}
const bool ConsoleCommands::floatArrayMatch(const std::string& command, const std::string& parsedCommand) {
	for (int i = 0; i < m_floatArrayCommands.size(); i++) {
		if (m_floatArrayCommands[i].command == parsedCommand) {
			std::vector<float> arr;
			size_t location = command.find(" ");
			while (location != std::string::npos) {
				size_t nextLocation = command.find(" ", location + 1);
				std::string section = command.substr(location + 1, nextLocation - location - 1);
				if (section.size() > 9)
					section = section.substr(0, 9);
				float value = std::stof(section);
				arr.emplace_back(value);
				location = nextLocation;
			}

			addLog(m_textField);
			addLog(m_floatArrayCommands[i].func(arr));
			m_textField = "";
			return true;
		}
	}
	return false;
}
const bool ConsoleCommands::stringMatch(const std::string& command, const std::string& parsedCommand) {
	for (int i = 0; i < m_stringCommands.size(); i++) {
		if (m_stringCommands[i].command == parsedCommand) {
			size_t location = command.find(" ");
			size_t size = command.find(" ", location + 1) - location - 1;
			std::string value = command.substr(location + 1, size);

			addLog(m_textField);
			addLog(m_stringCommands[i].func(value));
			m_textField = "";
			return true;
		}
	}
	return false;
}

int ConsoleCommands::inputCallback(ImGuiTextEditCallbackData* data) {
	switch (data->EventFlag) {
	case ImGuiInputTextFlags_CallbackCompletion:
		{
			// Example of TEXT COMPLETION
			// Locate beginning of current word
			const char* word_end = data->Buf + data->CursorPos;
			const char* word_start = word_end;
			while (word_start > data->Buf) {
				const char c = word_start[-1];
				if (c == ' ' || c == '\t' || c == ',' || c == ';')
					break;
				word_start--;
			}
			// Build a list of candidates
			ImVector<const char*> candidates;
			for (int i = 0; i < m_commandNames.size(); i++)
				if (_strnicmp(m_commandNames[i].first.c_str(), word_start, (int)(word_end - word_start)) == 0)
					candidates.push_back(m_commandNames[i].first.c_str());
			if (candidates.Size == 0) {
				// No match
				std::ostringstream stringStream;
				std::string s(word_start, word_end - word_start);
				stringStream << "No match for " << s << "\n";
				addLog(stringStream.str());
			} else if (candidates.Size == 1) {
				// Single match. Delete the beginning of the word and replace it entirely so we've got nice casing
				data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
				data->InsertChars(data->CursorPos, candidates[0]);
				data->InsertChars(data->CursorPos, " ");
			} else {
				// Multiple matches. Complete as much as we can, so inputing "C" will complete to "CL" and display "CLEAR" and "CLASSIFY"
				int match_len = (int)(word_end - word_start);
				for (;;) {
					int c = 0;
					bool all_candidates_matches = true;
					for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
						if (i == 0)
							c = toupper(candidates[i][match_len]);
						else if (c == 0 || c != toupper(candidates[i][match_len]))
							all_candidates_matches = false;
					if (!all_candidates_matches)
						break;
					match_len++;
				}
				if (match_len > 0) {
					data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
					data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
				}
				// List matches
				addLog("Possible matches:\n");
				for (int i = 0; i < candidates.Size; i++) {
					std::ostringstream stringStream;
					stringStream << "- " << candidates[i] << "\n";
					addLog(stringStream.str());
				}
			}
			break;
		}
	case ImGuiInputTextFlags_CallbackHistory:
		{
			// Example of HISTORY
			const int prev_history_pos = m_historyPos;
			if (data->EventKey == ImGuiKey_UpArrow) {
				if (m_historyPos == -1) {
					m_historyPos = (int)m_commandHistory.size() - 1;
				} else if (m_historyPos > 0) {
					m_historyPos--;
				}
			} else if (data->EventKey == ImGuiKey_DownArrow) {
				if (m_historyPos != -1) {
					if (++m_historyPos >= m_commandHistory.size()) {
						m_historyPos = -1;
					}
				}
			}
			// A better implementation would preserve the data on the current input line along with cursor position.
			if (prev_history_pos != m_historyPos) {
				data->CursorPos = data->SelectionStart = data->SelectionEnd = data->BufTextLen = (int)_snprintf_s(data->Buf, (size_t)data->BufSize, (size_t)data->BufSize, "%s", (m_historyPos >= 0) ? m_commandHistory[m_historyPos].c_str() : "");
				data->BufDirty = true;
			}
		}
	}

	return 0;
}


