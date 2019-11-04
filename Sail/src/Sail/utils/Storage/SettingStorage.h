#pragma once
#include <map>

class SettingStorage {
public:
	SettingStorage(const std::string& filename);
	~SettingStorage();

	bool loadFromFile(const std::string& filename);
	bool saveToFile(const std::string& filename);

	class Setting {
	public:
		struct Option {
			std::string name;
			float value;
		};

		Setting();
		Setting(const unsigned int selected, std::vector<Setting::Option>& options);
		~Setting();
		unsigned int selected;
		std::vector<Option> options;
		

	};
	// settings[area][setting] = profit; 
	std::map<std::string, std::map<std::string, Setting>> predeterminedSettings;
	std::map<std::string, std::map<std::string, float>> dynamicSettings;



private:

	void createDefaultStructure();
	void createDefaultGraphics();
	void createDefaultSound();
	void createDefaultMisc();

};