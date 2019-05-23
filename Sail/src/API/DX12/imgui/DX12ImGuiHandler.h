#pragma once
#include "Sail/api/ImGuiHandler.h"

class DX12ImGuiHandler : public ImGuiHandler {
public:
	DX12ImGuiHandler();
	~DX12ImGuiHandler();
	virtual void init() override;
	virtual void begin() override;
	virtual void end() override;
};