#pragma once

#include <map>
#include <string>
#include <functional>
class Settings {
public:
	enum Type {
		Graphics_MSAA,
		Graphics_SSAO,
		Graphics_DXR,
	};

public:
	Settings();

	void set(Settings::Type type, bool value);
	void set(Settings::Type type, int value);
	void set(Settings::Type type, float value);
	void set(Settings::Type type, const std::string& value);

	bool getBool(Settings::Type type);
	int getInt(Settings::Type type);
	float getFloat(Settings::Type type);
	const std::string& getString(Settings::Type type);

private:
	std::map<Type, std::string> m_settingsMap;
	std::map<Type, std::function<bool()>> m_requirementsMap;
};