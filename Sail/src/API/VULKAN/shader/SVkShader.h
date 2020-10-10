#pragma once

#include "Sail/api/shader/Shader.h"
#include "vulkan/vulkan_core.h"
#include "SVkSampler.h"
#include "../resources/SVkTexture.h"
#include "SVkPipelineStateObject.h"

class SVkAPI;

class SVkShader : public Shader {
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

	// This will write to each RendererCommand.materialIndex
	// It will also update material texture indices
	// Should be called whenever a list of renderCommands is available
	virtual void updateDescriptorsAndMaterialIndices(std::vector<Renderer::RenderCommand>& renderCommands, const Environment& environment, const PipelineStateObject* pso, void* cmd = nullptr) override;

	// "Manual" method to updates descriptors
	void updateDescriptors(const Descriptors& descriptors, const SVkPipelineStateObject* pso);

	const VkPipelineLayout& getPipelineLayout() const;

	void setRenderPass(const VkRenderPass& renderPass);
	const VkRenderPass& getRenderPass() const;

protected:
	virtual void* compileShader(const std::string& source, const std::string& filepath, ShaderComponent::BIND_SHADER shaderType) override;

private:
	virtual bool setConstantDerived(const std::string& name, const void* data, uint32_t size, ShaderComponent::BIND_SHADER bindShader, uint32_t byteOffset, void* cmdList) override;

	friend SVkPipelineStateObject;
	// This should only be called from a SVkPipelineStateObject instance
	void createDescriptorSet(std::vector<VkDescriptorSet>& outDescriptorSet) const;

private:
	SVkAPI* m_context;

	VkDescriptorSetLayout m_descriptorSetLayout;
	VkPipelineLayout m_pipelineLayout;

	VkRenderPass m_renderPass;

	// Textures used while waiting for the proper textures to finish uploading to the GPU
	SVkTexture& m_missingTexture;
	SVkTexture& m_missingTextureCube;

};