#pragma once

#include <functional>
#include "IconsFontAwesome4.h"

class SailGuiWindow {
public:
	virtual void addProperty(const char* label, std::function<void()> prop) = 0;

	virtual void setOption(const std::string& optionName, bool value) { };
	virtual void newSection(const std::string& title) { };

	// Returns an empty string if dialog is canceled
	// TODO: make method cross platform
	std::string openFileDialog(LPCWSTR filter = L"All Files (*.*)\0*.*\0", HWND owner = NULL);
	void limitStringLength(std::string& str, int maxLength = 20);
};