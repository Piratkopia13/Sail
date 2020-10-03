#pragma once

#include "Sail/api/shader/Shader.h"
#include "Sail/events/Events.h"
#include "vulkan/vulkan_core.h"
#include "SVkSampler.h"
#include "../resources/SVkTexture.h"
#include "SVkPipelineStateObject.h"

class SVkAPI;

class SVkShader : public Shader, public IEventListener {
public:
	struct ImageDescriptor {
		std::vector<VkDescriptorImageInfo> infos;
		std::string name;
	};
	struct BufferDescriptor {
		std::vector<VkDescriptorBufferInfo> infos;
		std::string name;
	};
	struct Descriptors {
		std::vector<ImageDescriptor> images;
		std::vector<BufferDescriptor> buffers;
	};

public:
	SVkShader(Shaders::ShaderSettings settings);
	~SVkShader();

	virtual void bind(void* cmdList) const override;
	virtual void recompile() override;

	virtual bool setTexture(const std::string& name, Texture* texture, void* cmdList = nullptr) override;
	virtual void setRenderableTexture(const std::string& name, RenderableTexture* texture, void* cmdList = nullptr) override;
	void setCBufferVar(const std::string& name, const void* data, unsigned int size, void* cmdList) override;
	bool trySetCBufferVar(const std::string& name, const void* data, unsigned int size, void* cmdList) override;

	bool onEvent(Event& event) override;

	// This will write to each RendererCommand.materialIndex
	// It will also update material texture indices
	// Should be called whenever a list of renderCommands is available
	void updateDescriptorsAndMaterialIndices(std::vector<Renderer::RenderCommand>& renderCommands, const SVkPipelineStateObject* pso);

	// "Manual" method to updates descriptors
	void updateDescriptors(const Descriptors& descriptors, const SVkPipelineStateObject* pso);

	const VkPipelineLayout& getPipelineLayout() const;

	void setRenderPass(const VkRenderPass& renderPass);
	const VkRenderPass& getRenderPass() const;

protected:
	virtual void* compileShader(const std::string& source, const std::string& filepath, ShaderComponent::BIND_SHADER shaderType) override;

private:
	bool trySetPushConstant(const std::string& name, const void* data, unsigned int size, void* cmdList);

	friend SVkPipelineStateObject;
	// This should only be called from a SVkPipelineStateObject instance
	void createDescriptorSet(std::vector<VkDescriptorSet>& outDescriptorSet) const;

private:
	SVkAPI* m_context;
	const unsigned int TEXTURE_ARRAY_DESCRIPTOR_COUNT = 128;
	const unsigned int MATERIAL_ARRAY_DESCRIPTOR_COUNT = 1024;

	VkDescriptorSetLayout m_descriptorSetLayout;
	VkPipelineLayout m_pipelineLayout;

	VkRenderPass m_renderPass;

	// Textures used while waiting for the proper textures to finish uploading to the GPU
	SVkTexture& m_missingTexture;
	SVkTexture& m_missingTextureCube;

	// Resources reset every frame
	// Since the same shader can be used for multiple PSOs, they need to be stored between every call to prepareToRender() this frame
	std::vector<Material*> m_uniqueMaterials;
	std::vector<SVkATexture*> m_uniqueTextures;
	std::vector<SVkATexture*> m_uniqueTextureCubes;
	unsigned int m_lastMaterialIndex = 0;
	unsigned int m_lastTextureIndex = 0;
	unsigned int m_lastTextureCubeIndex = 0;
};