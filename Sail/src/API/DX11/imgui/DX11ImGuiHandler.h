#pragma once
#include "Sail/api/ImGuiHandler.h"

class DX11ImGuiHandler : public ImGuiHandler {
public:
	DX11ImGuiHandler();
	~DX11ImGuiHandler();

	virtual void init() override;
	virtual void begin() override;
	virtual void end() override;
	virtual ImTextureID getTextureID(Texture* texture) override;
};