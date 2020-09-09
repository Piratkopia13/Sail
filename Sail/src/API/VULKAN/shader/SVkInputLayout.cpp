#include "pch.h"
#include "SVkInputLayout.h"
#include "Sail/utils/Utils.h"

InputLayout* InputLayout::Create() {
	return SAIL_NEW SVkInputLayout();
}

SVkInputLayout::SVkInputLayout()
	: m_vertexInputInfo()
{

}

SVkInputLayout::~SVkInputLayout() {

}

void SVkInputLayout::pushFloat(InputType inputType, const char* semanticName, unsigned int semanticIndex, unsigned int inputSlot, int alignedByteOffset, InputClassification inputSlotClass, unsigned int instanceDataStepRate) {
	assert(inputSlotClass != InputLayout::PER_INSTANCE_DATA && "Per instance data not implemented in Vulkan");
	push(VK_FORMAT_R32_SFLOAT, sizeof(float), inputSlot);
	InputLayout::pushFloat(inputType, semanticName, semanticIndex, inputSlot, alignedByteOffset, inputSlotClass, instanceDataStepRate);
}

void SVkInputLayout::pushVec2(InputType inputType, const char* semanticName, unsigned int semanticIndex, unsigned int inputSlot, int alignedByteOffset, InputClassification inputSlotClass, unsigned int instanceDataStepRate) {
	assert(inputSlotClass != InputLayout::PER_INSTANCE_DATA && "Per instance data not implemented in Vulkan");
	push(VK_FORMAT_R32G32_SFLOAT, sizeof(glm::vec2), inputSlot);
	InputLayout::pushVec2(inputType, semanticName, semanticIndex, inputSlot, alignedByteOffset, inputSlotClass, instanceDataStepRate);
}

void SVkInputLayout::pushVec3(InputType inputType, const char* semanticName, unsigned int semanticIndex, unsigned int inputSlot, int alignedByteOffset, InputClassification inputSlotClass, unsigned int instanceDataStepRate) {
	assert(inputSlotClass != InputLayout::PER_INSTANCE_DATA && "Per instance data not implemented in Vulkan");
	push(VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3), inputSlot);
	InputLayout::pushVec3(inputType, semanticName, semanticIndex, inputSlot, alignedByteOffset, inputSlotClass, instanceDataStepRate);
}

void SVkInputLayout::pushVec4(InputType inputType, const char* semanticName, unsigned int semanticIndex, unsigned int inputSlot, int alignedByteOffset, InputClassification inputSlotClass, unsigned int instanceDataStepRate) {
	assert(inputSlotClass != InputLayout::PER_INSTANCE_DATA && "Per instance data not implemented in Vulkan");
	push(VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec4), inputSlot);
	InputLayout::pushVec4(inputType, semanticName, semanticIndex, inputSlot, alignedByteOffset, inputSlotClass, instanceDataStepRate);
}

void SVkInputLayout::create(void* vertexShaderBlob) {
	m_vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	m_vertexInputInfo.vertexBindingDescriptionCount = m_bindingDescriptions.size();
	m_vertexInputInfo.pVertexBindingDescriptions = m_bindingDescriptions.data();
	m_vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_attributeDescriptions.size());
	m_vertexInputInfo.pVertexAttributeDescriptions = m_attributeDescriptions.data();
}

void SVkInputLayout::bind() const {
	// Do nothing
	// Already bound through the pipeline state
}

const VkPipelineVertexInputStateCreateInfo& SVkInputLayout::getCreateInfo() const {
	return m_vertexInputInfo;
}

int SVkInputLayout::convertInputClassification(InputClassification inputSlotClass) {
	assert(false);

	switch (inputSlotClass) {
	case InputLayout::PER_VERTEX_DATA:
		return 0;
		break;
	case InputLayout::PER_INSTANCE_DATA:
		return 0;
		break;
	default:
		Logger::Error("Invalid input classifier specified");
		return 0;
		break;
	}
}

void SVkInputLayout::push(VkFormat format, unsigned int typeSize, unsigned int location) {
	static unsigned int binding = 0;
	m_attributeDescriptions.emplace_back(VkVertexInputAttributeDescription{static_cast<uint32_t>(location), binding, format, 0});
	m_bindingDescriptions.emplace_back(VkVertexInputBindingDescription{ binding++, typeSize, VK_VERTEX_INPUT_RATE_VERTEX });

	VertexSize += typeSize;
}
