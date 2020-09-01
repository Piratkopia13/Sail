#pragma once

#include "../VkAPI.h"
#include "Sail/api/shader/PipelineStateObject.h"
//#include "DXILShaderCompiler.h"

class VkPipelineStateObject : public PipelineStateObject {
public:
	VkPipelineStateObject(Shader* shader, unsigned int attributesHash);

	virtual bool bind(void* cmdList) override;
	
private:
	void createGraphicsPipelineState();
	void createComputePipelineState();

private:
	VkAPI* m_context;


};