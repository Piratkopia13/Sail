#pragma once

#include <memory>
#include "imgui.h"

//inline std::string className(const std::string& prettyFunction) {
//	size_t colons = prettyFunction.find("::");
//	if (colons == std::string::npos)
//		return "::";
//	size_t begin = prettyFunction.substr(0, colons).rfind(" ") + 1;
//	size_t end = colons - begin;
//
//	return prettyFunction.substr(begin, end);
//}
//
//#define __CLASS_NAME__ className(__PRETTY_FUNCTION__)

// Creates a unique id for each class which derives from component
// This method uses the gcc defined __COUNTER__ macro that increments with every use
#define SAIL_COMPONENT \
	static int getStaticID() { return __COUNTER__; } \
	virtual int getID() override { return getStaticID(); } \
	virtual std::string getName() override { std::string name(__FUNCTION__); return name.substr(0, name.length() - 9); }

class SailGuiWindow;

class Component {
public:
	typedef std::unique_ptr<Component> Ptr;
	typedef std::shared_ptr<Component> SPtr;
public:
	Component() {}
	virtual ~Component() {}
	virtual void renderEditorGui(SailGuiWindow* window) { ImGui::Text("This component has no properties"); };

	// These methods will be overriden by the SAIL_COMPONENT macro
	virtual int getID() = 0;
	virtual std::string getName() = 0;

private:
};

