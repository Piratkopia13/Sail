#include "pch.h"
#include "DX12InputLayout.h"

InputLayout* InputLayout::Create() {
	return new DX12InputLayout();
}

DX12InputLayout::DX12InputLayout() {

}

DX12InputLayout::~DX12InputLayout() {

}

void DX12InputLayout::pushFloat(InputType inputType, LPCSTR semanticName, UINT semanticIndex, InputClassification inputSlotClass /*= PER_VERTEX_DATA*/, UINT instanceDataStepRate /*= 0*/) {
	throw std::logic_error("The method or operation is not implemented.");
}

void DX12InputLayout::pushVec2(InputType inputType, LPCSTR semanticName, UINT semanticIndex, InputClassification inputSlotClass /*= PER_VERTEX_DATA*/, UINT instanceDataStepRate /*= 0*/) {
	throw std::logic_error("The method or operation is not implemented.");
}

void DX12InputLayout::pushVec3(InputType inputType, LPCSTR semanticName, UINT semanticIndex, InputClassification inputSlotClass /*= PER_VERTEX_DATA*/, UINT instanceDataStepRate /*= 0*/) {
	throw std::logic_error("The method or operation is not implemented.");
}

void DX12InputLayout::pushVec4(InputType inputType, LPCSTR semanticName, UINT semanticIndex, InputClassification inputSlotClass /*= PER_VERTEX_DATA*/, UINT instanceDataStepRate /*= 0*/) {
	throw std::logic_error("The method or operation is not implemented.");
}

void DX12InputLayout::create(void* vertexShaderBlob) {
	throw std::logic_error("The method or operation is not implemented.");
}

void DX12InputLayout::bind() const {
	throw std::logic_error("The method or operation is not implemented.");
}

const D3D12_INPUT_LAYOUT_DESC& DX12InputLayout::getDesc() const {
	return m_inputLayoutDesc;
}

int DX12InputLayout::convertInputClassification(InputClassification inputSlotClass) {
	throw std::logic_error("The method or operation is not implemented.");
}
