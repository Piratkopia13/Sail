#include "pch.h"
#include "Settings.h"
#include <assert.h>
#include "Application.h"

Settings::Settings() {
	set(Settings::Graphics_MSAA, 8); // 8x MSAA as default
	//set(Settings::Graphics_MSAA, 1);

	m_requirementsMap[Graphics_DXR] = [] { return Application::getInstance()->getAPI()->supportsFeature(GraphicsAPI::RAYTRACING); };
}

void Settings::set(Settings::Type type, bool value) {
	if (m_requirementsMap.find(type) == m_requirementsMap.end() || m_requirementsMap[type]())
		m_settingsMap[type] = std::to_string(value);
}

void Settings::set(Type type, int value) {
	if (m_requirementsMap.find(type) == m_requirementsMap.end() || m_requirementsMap[type]())
		m_settingsMap[type] = std::to_string(value);
}

void Settings::set(Type type, float value) {
	if (m_requirementsMap.find(type) == m_requirementsMap.end() || m_requirementsMap[type]())
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
