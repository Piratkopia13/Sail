#pragma once

#include "Sail/api/shader/InputLayout.h"
#include "vulkan/vulkan_core.h"

class SVkInputLayout : public InputLayout {
public:
	SVkInputLayout();
	~SVkInputLayout();

	virtual void pushFloat(InputType inputType, const char* semanticName, unsigned int semanticIndex, unsigned int inputSlot, int alignedByteOffset = -1, InputClassification inputSlotClass = PER_VERTEX_DATA, unsigned int instanceDataStepRate = 0) override;
	virtual void pushVec2(InputType inputType, const char* semanticName, unsigned int semanticIndex, unsigned int inputSlot, int alignedByteOffset = -1, InputClassification inputSlotClass = PER_VERTEX_DATA, unsigned int instanceDataStepRate = 0) override;
	virtual void pushVec3(InputType inputType, const char* semanticName, unsigned int semanticIndex, unsigned int inputSlot, int alignedByteOffset = -1, InputClassification inputSlotClass = PER_VERTEX_DATA, unsigned int instanceDataStepRate = 0) override;
	virtual void pushVec4(InputType inputType, const char* semanticName, unsigned int semanticIndex, unsigned int inputSlot, int alignedByteOffset = -1, InputClassification inputSlotClass = PER_VERTEX_DATA, unsigned int instanceDataStepRate = 0) override;
	virtual void create(void* vertexShaderBlob) override;
	virtual void bind() const override;

	const VkPipelineVertexInputStateCreateInfo& getCreateInfo() const;

protected:
	virtual int convertInputClassification(InputClassification inputSlotClass) override;

private:
	void push(VkFormat format, unsigned int typeSize, unsigned int location);

private:
	std::vector<VkVertexInputBindingDescription> m_bindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> m_attributeDescriptions;

	VkPipelineVertexInputStateCreateInfo m_vertexInputInfo;
};