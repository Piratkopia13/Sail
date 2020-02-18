#include "pch.h"
#include "Settings.h"
#include <assert.h>

void Settings::set(Settings::Type type, bool value) {
	m_settingsMap[type] = std::to_string(value);
}

void Settings::set(Type type, int value) {
	m_settingsMap[type] = std::to_string(value);
}

void Settings::set(Type type, float value) {
	m_settingsMap[type] = std::to_string(value);
}

void Settings::set(Type type, const std::string& value) {
	m_settingsMap[type] = value;
}

bool Settings::getBool(Type type) {
	return m_settingsMap[type] == "1";
}

int Settings::getInt(Type type) {
	return std::stoi(m_settingsMap[type]);
}

float Settings::getFloat(Type type) {
	assert(false);
	return 0.f;
}

const std::string& Settings::getString(Type type) {
	assert(false);
	return "";
}
