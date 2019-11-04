#include "pch.h"
#include "SettingStorage.h"
#include "../Utils.h"

#pragma region OTIONSSTORAGE
SettingStorage::SettingStorage(const std::string& file) {
	if (!loadFromFile(file)) {
		Logger::Log("Could not load options from " + file);
		assert(0);
	}

}

SettingStorage::~SettingStorage() {


}

bool SettingStorage::loadFromFile(const std::string& filename) {
	std::string file = Utils::readFile(filename);
	std::string currentArea = "";
	while (file != "") {
		unsigned int newline = file.find("\n");
		std::string line = file.substr(0, newline);
		file = file.substr(newline+1, std::string::npos);

		if (line[0] == '#') {
			std::string name = line.substr(1, std::string::npos);
			currentArea = name;
		}
		else if(line != ""){
			int divider = line.find(":");
			std::string name = line.substr(0, divider);
			std::string temp = line.substr(divider, std::string::npos);
			float selection = std::stof(temp);

			if (predeterminedSettings[currentArea].find(name) == predeterminedSettings[currentArea].end()) {
				predeterminedSettings[currentArea][name].selected = (unsigned int)selection;
			}
			if (dynamicSettings[currentArea].find(name) == dynamicSettings[currentArea].end()) {
				dynamicSettings[currentArea][name] = selection;
			}
		}
	}

	return false;
}

bool SettingStorage::saveToFile(const std::string& filename) {
	return false;
}

void SettingStorage::createDefaultStructure() {
	createDefaultGraphics();
	createDefaultSound();
	createDefaultMisc();
}

void SettingStorage::createDefaultGraphics() {
	predeterminedSettings["graphics"] = std::map<std::string, Setting>();
	predeterminedSettings["graphics"]["fullscreen"] = Setting(0, std::vector<Setting::Option>({ 
		{ "on", 1.0f }, 
		{ "off",0.0f } 
	}));
	predeterminedSettings["graphics"]["bloom"] = Setting(0, std::vector<Setting::Option>({
		{ "on", 0.0f },
		{ "off",1.0f }
	}));
	predeterminedSettings["graphics"]["shadows"] = Setting(0, std::vector<Setting::Option>({
		{ "off", 0.0f },
		{ "hard",1.0f },
		{ "soft",2.0f }
	}));
}

void SettingStorage::createDefaultSound() {
	dynamicSettings["sound"] = std::map<std::string, float>();
	dynamicSettings["sound"]["global"]  = 1.0f;
	dynamicSettings["sound"]["music"]   = 1.0f;
	dynamicSettings["sound"]["effects"] = 1.0f;
	dynamicSettings["sound"]["voices"]  = 1.0f;
}

void SettingStorage::createDefaultMisc() {
	predeterminedSettings["misc"] = std::map<std::string, Setting>();
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

#pragma endregion