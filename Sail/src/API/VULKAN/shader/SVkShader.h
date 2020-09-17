#pragma once

#include "Sail/api/shader/Shader.h"
#include "Sail/events/Events.h"
#include "vulkan/vulkan_core.h"
#include "SVkSampler.h"
#include "../resources/SVkTexture.h"

class SVkAPI;

class SVkShader : public Shader, public IEventListener {
public:
	SVkShader(Shaders::ShaderSettings settings);
	~SVkShader();

	virtual void bind(void* cmdList) const override;

	virtual void* compileShader(const std::string& source, const std::string& filepath, ShaderComponent::BIND_SHADER shaderType) override;

	virtual bool setTexture(const std::string& name, Texture* texture, void* cmdList = nullptr) override;
	virtual void setRenderableTexture(const std::string& name, RenderableTexture* texture, void* cmdList = nullptr) override;
	void setCBufferVar(const std::string& name, const void* data, unsigned int size, void* cmdList) override;
	bool trySetCBufferVar(const std::string& name, const void* data, unsigned int size, void* cmdList) override;

	bool onEvent(Event& event) override;

	// This will write to each RendererCommand.materialIndex
	void prepareToRender(std::vector<Renderer::RenderCommand>& renderCommands);
	//void updateDescriptorSet(void* cmdList);
	const VkPipelineLayout& getPipelineLayout() const;

private:
	bool trySetPushConstant(const std::string& name, const void* data, unsigned int size, void* cmdList);

private:
	SVkAPI* m_context;
	const unsigned int TEXTURE_ARRAY_DESCRIPTOR_COUNT = 128;
	const unsigned int MATERIAL_ARRAY_DESCRIPTOR_COUNT = 1024;

	VkDescriptorSetLayout m_descriptorSetLayout;
	std::vector<VkDescriptorSet> m_descriptorSets;
	VkPipelineLayout m_pipelineLayout;

	std::vector<VkDescriptorImageInfo> m_imageInfos;

	// Texture used while waiting for the proper texture to finish uploading to the GPU
	SVkTexture& m_missingTexture;
};