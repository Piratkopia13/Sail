#pragma once
#include "Sail/api/ImGuiHandler.h"
#include "../resources/DescriptorHeap.h"

class DX12ImGuiHandler : public ImGuiHandler {
public:
	DX12ImGuiHandler();
	~DX12ImGuiHandler();
	virtual void init() override;
	virtual void begin() override;
	virtual void end() override;

private:
	DX12API* m_context;
	std::unique_ptr<DescriptorHeap> m_descHeap;
	DX12API::Command m_command;

};