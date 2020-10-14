#pragma once

#include "imgui.h"

class SailGuiWindow;

class Component {
public:
	Component() {}
	virtual ~Component() {}
	virtual void renderEditorGui(SailGuiWindow* window) { ImGui::Text("This component has no properties"); };

};

