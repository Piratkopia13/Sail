#include "pch.h"
#include "VkInputLayout.h"
#include "Sail/utils/Utils.h"

InputLayout* InputLayout::Create() {
	return SAIL_NEW VkInputLayout();
}

VkInputLayout::VkInputLayout() {

}

VkInputLayout::~VkInputLayout() {

}

void VkInputLayout::pushFloat(InputType inputType, const char* semanticName, unsigned int semanticIndex, unsigned int inputSlot, int alignedByteOffset, InputClassification inputSlotClass, unsigned int instanceDataStepRate) {
	assert(false);
	InputLayout::pushFloat(inputType, semanticName, semanticIndex, inputSlot, alignedByteOffset, inputSlotClass, instanceDataStepRate);
}

void VkInputLayout::pushVec2(InputType inputType, const char* semanticName, unsigned int semanticIndex, unsigned int inputSlot, int alignedByteOffset, InputClassification inputSlotClass, unsigned int instanceDataStepRate) {
	assert(false);
	InputLayout::pushVec2(inputType, semanticName, semanticIndex, inputSlot, alignedByteOffset, inputSlotClass, instanceDataStepRate);
}

void VkInputLayout::pushVec3(InputType inputType, const char* semanticName, unsigned int semanticIndex, unsigned int inputSlot, int alignedByteOffset, InputClassification inputSlotClass, unsigned int instanceDataStepRate) {
	assert(false);
	InputLayout::pushVec3(inputType, semanticName, semanticIndex, inputSlot, alignedByteOffset, inputSlotClass, instanceDataStepRate);
}

void VkInputLayout::pushVec4(InputType inputType, const char* semanticName, unsigned int semanticIndex, unsigned int inputSlot, int alignedByteOffset, InputClassification inputSlotClass, unsigned int instanceDataStepRate) {
	assert(false);
	InputLayout::pushVec4(inputType, semanticName, semanticIndex, inputSlot, alignedByteOffset, inputSlotClass, instanceDataStepRate);
}

void VkInputLayout::create(void* vertexShaderBlob) {
	assert(false);
}

void VkInputLayout::bind() const {
	// Do nothing
	// Already bound through the pipeline state
}

int VkInputLayout::convertInputClassification(InputClassification inputSlotClass) {
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