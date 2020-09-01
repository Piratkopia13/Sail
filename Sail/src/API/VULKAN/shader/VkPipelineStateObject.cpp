#include "pch.h"
#include "VkPipelineStateObject.h"
#include "Sail/Application.h"
//#include "VkInputLayout.h"
#include "../VkAPI.h"
#include "Sail/api/shader/Shader.h"
#include "../resources/VkTexture.h"

PipelineStateObject* PipelineStateObject::Create(Shader* shader, unsigned int attributesHash) {
	return SAIL_NEW VkPipelineStateObject(shader, attributesHash);
}

VkPipelineStateObject::VkPipelineStateObject(Shader* shader, unsigned int attributesHash)
	: PipelineStateObject(shader, attributesHash)
{
	assert(false);
}

bool VkPipelineStateObject::bind(void* cmdList) {
	assert(false);

	return true;
}

void VkPipelineStateObject::createGraphicsPipelineState() {
	assert(false);
}

void VkPipelineStateObject::createComputePipelineState() {
	assert(false);
}