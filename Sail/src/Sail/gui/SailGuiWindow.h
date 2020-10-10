#pragma once

#include <functional>
#include "IconsFontAwesome4.h"

class SailGuiWindow {
public:
	SailGuiWindow();

	virtual void addProperty(const char* label, std::function<void()> prop);
	virtual void setOption(const std::string& optionName, bool value);
	virtual void newSection(const std::string& title);

	void enableColumns(float labelWidth = 75.f);
	void disableColumns();

	// Static helper methods

	// Returns an empty string if dialog is canceled
	// TODO: make method cross platform
	static std::string OpenFileDialog(const wchar_t* filter = L"All Files (*.*)\0*.*\0");
	static void LimitStringLength(std::string& str, int maxLength = 20);
	static bool DrawSplitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f);

protected:
	// Always call this at the beginning the gui rendering
	void newFrame();

protected:
	bool setNextPropWidth;
	float propWidth;
	int propID;

};