#pragma once

#include "../SVkAPI.h"
#include "Sail/api/shader/PipelineStateObject.h"

class SVkShader;

class SVkPipelineStateObject : public PipelineStateObject {
public:
	SVkPipelineStateObject(Shader* shader, unsigned int attributesHash);
	~SVkPipelineStateObject();

	virtual bool bind(void* cmdList) override;
	const VkDescriptorSet& getDescriptorSet() const;
	const VkPipeline& getPipeline() const;
	
private:
	void createGraphicsPipelineState();
	void createComputePipelineState();
	void createRaytracingPipelineState();

private:
	SVkAPI* m_context;

	SVkShader* m_vkShader;
	VkPipeline m_pipeline;
	std::vector<VkDescriptorSet> m_descriptorSets;

};