#include "pch.h"
#include "SettingStorage.h"
#include "../Utils.h"
#include "..//Regex/Regex.h"

#pragma region OTIONSSTORAGE
SettingStorage::SettingStorage(const std::string& file) {
	createApplicationDefaultStructure();
	if (!loadFromFile(file)) {
		assert(0);
	}
	if (!saveToFile(file)) {
		assert(0);
	}
	createGameDefaultStructure();
}

SettingStorage::~SettingStorage() {


}

bool SettingStorage::loadFromFile(const std::string& filename) {
	std::string file = Utils::readFile(filename);
	std::string currentArea = "";
	while (file != "") {
		// get a new line from the buffer
		int newline = file.find("\n");
		std::string line = "";
		if (newline != std::string::npos) {
			line = file.substr(0, newline);
			file = file.substr(newline+1, std::string::npos);
		}
		else {
			line = file;
			file = "";
		}

		// Start new section
		if (line[0] == '#') {
			
			currentArea = line.substr(1, std::string::npos);
		}
		// import new value
		else if(line != "" && currentArea != ""){
			if (Reg::Setting.match(line.c_str()) == line.size()) {
				int divider = line.find(":");
				std::string name = line.substr(0, divider);
				std::string temp = line.substr(divider + 1, std::string::npos);
				if (applicationSettingsStatic.find(currentArea) != applicationSettingsStatic.end()) {
					if (applicationSettingsStatic[currentArea].find(name) != applicationSettingsStatic[currentArea].end()) {
						int selection = std::stoi(temp);
						applicationSettingsStatic[currentArea][name].setSelected((unsigned int)selection);
					}
				}
				if (applicationSettingsDynamic.find(currentArea) != applicationSettingsDynamic.end()) {
					if (applicationSettingsDynamic[currentArea].find(name) != applicationSettingsDynamic[currentArea].end()) {
						float value = std::stof(temp);
						applicationSettingsDynamic[currentArea][name].value = value;
					}
				}
			}
		}
	}
	return true;
}

bool SettingStorage::saveToFile(const std::string& filename) {

	std::string output = "";
	for (auto const& [areaKey, setting]: applicationSettingsStatic) {
		output += "#" + areaKey + "\n";
		for (auto const& [settingKey, option] : setting) {
			output += settingKey + ":" + std::to_string(option.selected) + "\n";
		}
		output += "\n";
	}
	for (auto const& [areaKey, setting] : applicationSettingsDynamic) {
		output += "#" + areaKey + "\n";
		for (auto const& [settingKey, option] : setting) {
			output += settingKey + ":" + std::to_string(option.value) + "\n";
		}
		output += "\n";
	}
	Utils::writeFileTrunc(filename, output);
 	return true;
}

SettingStorage::WantedType SettingStorage::matchType(const std::string& value) {
	if (Reg::Number.match(value.c_str()) == value.size()) {
		return WantedType::INT;
	} 
	else if (Reg::DecimalNumber.match(value.c_str()) == value.size()) {
		return WantedType::FLOAT;
	}
	else {
		return WantedType::BOOL;
	}
}

void SettingStorage::createApplicationDefaultStructure() {
	createApplicationDefaultGraphics();
	createApplicationDefaultSound();
	createApplicationDefaultMisc();
}

void SettingStorage::createApplicationDefaultGraphics() {
	applicationSettingsStatic["graphics"] = std::map<std::string, Setting>();
	applicationSettingsStatic["graphics"]["fullscreen"] = Setting(1, std::vector<Setting::Option>({
		{ "on", 1.0f }, 
		{ "off",0.0f } 
	}));

	applicationSettingsStatic["graphics"]["bloom"] = Setting(0, std::vector<Setting::Option>({
		{ "on", 0.0f },
		{ "off",1.0f }
	}));
	applicationSettingsStatic["graphics"]["shadows"] = Setting(0, std::vector<Setting::Option>({
		{ "off", 0.0f },
		{ "hard",1.0f },
		{ "soft",2.0f }
	}));
}
void SettingStorage::createApplicationDefaultSound() {
	applicationSettingsDynamic["sound"] = std::map<std::string, DynamicSetting>();
	applicationSettingsDynamic["sound"]["global"]  = DynamicSetting(1.0f, 0.0f, 1.0f);
	applicationSettingsDynamic["sound"]["music"]   = DynamicSetting(1.0f, 0.0f, 1.0f);
	applicationSettingsDynamic["sound"]["effects"] = DynamicSetting(1.0f, 0.0f, 1.0f);
	applicationSettingsDynamic["sound"]["voices"]  = DynamicSetting(1.0f, 0.0f, 1.0f);
}
void SettingStorage::createApplicationDefaultMisc() {
	applicationSettingsStatic["misc"] = std::map<std::string, Setting>();
}

void SettingStorage::createGameDefaultStructure() {
	createGameDefaultMap();

}

void SettingStorage::createGameDefaultMap() {	
	gameSettingsDynamic["map"] = std::map<std::string, DynamicSetting>();
	gameSettingsDynamic["map"]["sizeX"] =   DynamicSetting(6.0f,	1.0f,	30.0f);
	gameSettingsDynamic["map"]["sizeY"] =   DynamicSetting(6.0f,	1.0f,	30.0f);
	gameSettingsDynamic["map"]["clutter"] = DynamicSetting(0.85f,	0.0f,	5.0f);
	gameSettingsDynamic["map"]["seed"] =    DynamicSetting(0.0f,	0.0f,	1000000.0f);
}






#pragma endregion








#pragma region OPTION
SettingStorage::Setting::Setting() {
	selected = 0;
}
SettingStorage::Setting::Setting(const unsigned int selectedOption, std::vector<Setting::Option>& asd) {
	selected = selectedOption;
	options = asd;
}

SettingStorage::Setting::~Setting() {
}

void SettingStorage::Setting::setSelected(const unsigned int selection) {
	selected = selection;
	if (selected > options.size()) {
		selected = options.size();
	}
}

#pragma endregion