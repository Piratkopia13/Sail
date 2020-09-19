#pragma once
#include "Sail/api/gui/ImGuiHandler.h"
#include "../SVkAPI.h"

class SVkImGuiHandler : public ImGuiHandler {
public:
	SVkImGuiHandler();
	~SVkImGuiHandler();

	virtual void init() override;
	virtual void begin() override;
	virtual void end() override;
	virtual ImTextureID getTextureID(Texture* texture) override;

private:
	SVkAPI* m_context;
	VkDescriptorPool m_descriptorPool;
	VkRenderPass m_renderPass;
};