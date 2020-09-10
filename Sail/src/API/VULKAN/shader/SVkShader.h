#pragma once

#include "Sail/api/shader/Shader.h"
#include "Sail/events/Events.h"
#include "vulkan/vulkan_core.h"

class SVkAPI;

class SVkShader : public Shader, public IEventListener {
public:
	SVkShader(Shaders::ShaderSettings settings);
	~SVkShader();

	virtual void bind(void* cmdList, uint32_t frameIndex) const override;

	virtual void* compileShader(const std::string& source, const std::string& filepath, ShaderComponent::BIND_SHADER shaderType) override;

	virtual bool setTexture(const std::string& name, Texture* texture, void* cmdList = nullptr) override;
	virtual void setRenderableTexture(const std::string& name, RenderableTexture* texture, void* cmdList = nullptr) override;
	void setCBufferVar(const std::string& name, const void* data, unsigned int size, void* cmdList) override;
	bool trySetCBufferVar(const std::string& name, const void* data, unsigned int size, void* cmdList) override;

	bool onEvent(Event& event) override;

	const VkPipelineLayout& getPipelineLayout() const;

private:
	bool trySetPushConstant(const std::string& name, const void* data, unsigned int size, void* cmdList);

private:
	SVkAPI* m_context;

	VkDescriptorSetLayout m_descriptorSetLayout;
	std::vector<VkDescriptorSet> m_descriptorSets;
	VkPipelineLayout m_pipelineLayout;
};