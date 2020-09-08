#pragma once

#include "../SVkAPI.h"
#include "Sail/api/shader/PipelineStateObject.h"

class SVkPipelineStateObject : public PipelineStateObject {
public:
	SVkPipelineStateObject(Shader* shader, unsigned int attributesHash);
	~SVkPipelineStateObject();

	virtual bool bind(void* cmdList, uint32_t frameIndex) override;
	
private:
	void createGraphicsPipelineState();
	void createComputePipelineState();

private:
	SVkAPI* m_context;

	VkPipeline m_pipeline;

};