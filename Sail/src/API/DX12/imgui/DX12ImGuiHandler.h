#pragma once
#include "Sail/api/gui/ImGuiHandler.h"
#include "../resources/DescriptorHeap.h"

class DX12Texture;

class DX12ImGuiHandler : public ImGuiHandler {
public:
	DX12ImGuiHandler();
	~DX12ImGuiHandler();

	virtual void init() override;
	virtual void begin() override;
	virtual void end() override;
	virtual ImTextureID getTextureID(Texture* texture) override;

private:
	DX12API* m_context;
	std::unique_ptr<DescriptorHeap> m_descHeap;
	DX12API::Command m_command;

	std::vector<DX12Texture*> m_texturesUsedThisFrame;

};