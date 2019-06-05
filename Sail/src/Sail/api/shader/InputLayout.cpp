#include "pch.h"
#include "InputLayout.h"
#include "Sail/Application.h"

InputLayout::InputLayout() {
	VertexSize = 0;
}

InputLayout::~InputLayout() {

}

void InputLayout::pushFloat(InputType inputType, LPCSTR semanticName, UINT semanticIndex, InputClassification inputSlotClass /*= PER_VERTEX_DATA*/, UINT instanceDataStepRate /*= 0*/) {
	InputOrder.push_back(inputType);
}

void InputLayout::pushVec2(InputType inputType, LPCSTR semanticName, UINT semanticIndex, InputClassification inputSlotClass /*= PER_VERTEX_DATA*/, UINT instanceDataStepRate /*= 0*/) {
	InputOrder.push_back(inputType);
}

void InputLayout::pushVec3(InputType inputType, LPCSTR semanticName, UINT semanticIndex, InputClassification inputSlotClass /*= PER_VERTEX_DATA*/, UINT instanceDataStepRate /*= 0*/) {
	InputOrder.push_back(inputType);
}

void InputLayout::pushVec4(InputType inputType, LPCSTR semanticName, UINT semanticIndex, InputClassification inputSlotClass /*= PER_VERTEX_DATA*/, UINT instanceDataStepRate /*= 0*/) {
	InputOrder.push_back(inputType);
}

const std::vector<InputLayout::InputType>& InputLayout::getOrderedInputs() const {
	return InputOrder;
}

UINT InputLayout::getVertexSize() const {
	return VertexSize;
}

UINT InputLayout::getInstanceSize() const {
	return InstanceSize;
}
