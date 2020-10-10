#pragma once
#include "imgui.h"
#include "IconsFontAwesome4.h"

class Texture;

class ImGuiHandler {
public:
	static ImGuiHandler* Create();

	ImGuiHandler() {}
	virtual ~ImGuiHandler() {}

	virtual void init() = 0;
	virtual void begin() = 0;
	virtual void end() = 0;

	virtual ImTextureID getTextureID(Texture* texture) = 0;

	void applySailStyle();
};