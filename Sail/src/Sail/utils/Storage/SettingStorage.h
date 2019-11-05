#pragma once
#include <map>

class SettingStorage {
public:
	SettingStorage(const std::string& filename);
	~SettingStorage();

	bool loadFromFile(const std::string& filename);
	bool saveToFile(const std::string& filename);
	enum WantedType {
		BOOL,
		INT,
		FLOAT
	};
	class Setting {
	public:
		struct Option {
			std::string name;
			float value;
		};

		Setting();
		Setting(const unsigned int selected, std::vector<Setting::Option>& options);
		~Setting();

		void setSelected(const unsigned int selection);

		unsigned int selected;
		std::vector<Option> options;
	};
	class DynamicSetting {
	public:
		DynamicSetting() : DynamicSetting(0.0f, 0.0f, 1.0f){}
		DynamicSetting(const float& _value, const float& _min, const float& _max) : 
			value(_value),
			minVal(_min),
			maxVal(_max)
		{
		}

		float value;
		float minVal;
		float maxVal;

	};


	// settings[area][setting].selected == current selected setting; 
	// settings[area][setting].options[selected].value == value of selected setting; 

	std::map<std::string, std::map<std::string, Setting>> applicationSettingsStatic;
	std::map<std::string, std::map<std::string, DynamicSetting>> applicationSettingsDynamic;


	std::map<std::string, std::map<std::string, Setting>> gameSettingsStatic;
	std::map<std::string, std::map<std::string, DynamicSetting>> gameSettingsDynamic;


private:

	WantedType matchType(const std::string& value);

private:

	void createApplicationDefaultStructure();
	void createApplicationDefaultGraphics();
	void createApplicationDefaultSound();
	void createApplicationDefaultMisc();

	void createGameDefaultStructure();
	void createGameDefaultMap();

};