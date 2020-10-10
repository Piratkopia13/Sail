#include "pch.h"
#include "../DX11API.h"
#include "DX11InputLayout.h"
#include "Sail/Application.h"

InputLayout* InputLayout::Create() {
	return SAIL_NEW DX11InputLayout();
}

DX11InputLayout::DX11InputLayout()
	: InputLayout()
{ }

DX11InputLayout::~DX11InputLayout() {
	Memory::SafeRelease(m_inputLayout);
}

void DX11InputLayout::pushFloat(InputType inputType, const char* semanticName, unsigned int semanticIndex, unsigned int inputSlot, int alignedByteOffset, InputClassification inputSlotClass, unsigned int instanceDataStepRate) {
	if (m_ied.size() == m_ied.capacity()) {
		m_ied.reserve(m_ied.size() + 1);
		InputOrder.reserve(InputOrder.size() + 1);
	}
	alignedByteOffset = (alignedByteOffset == -1) ? (m_ied.size() == 0) ? 0 : D3D11_APPEND_ALIGNED_ELEMENT : alignedByteOffset;
	DXGI_FORMAT format = DXGI_FORMAT_R32_FLOAT;;
	auto convertedInputClass = (D3D11_INPUT_CLASSIFICATION)convertInputClassification(inputSlotClass);
	m_ied.push_back({ semanticName, semanticIndex, format, inputSlot, (UINT)alignedByteOffset, convertedInputClass, instanceDataStepRate });

	UINT typeSize = sizeof(float);
	if (convertedInputClass == D3D11_INPUT_PER_INSTANCE_DATA)
		InstanceSize += typeSize;
	else
		VertexSize += typeSize;

	InputLayout::pushFloat(inputType, semanticName, semanticIndex, inputSlotClass, instanceDataStepRate);
}
void DX11InputLayout::pushVec2(InputType inputType, const char* semanticName, unsigned int semanticIndex, unsigned int inputSlot, int alignedByteOffset, InputClassification inputSlotClass, unsigned int instanceDataStepRate) {
	if (m_ied.size() == m_ied.capacity()) {
		m_ied.reserve(m_ied.size() + 1);
		InputOrder.reserve(InputOrder.size() + 1);
	}
	alignedByteOffset = (alignedByteOffset == -1) ? (m_ied.size() == 0) ? 0 : D3D11_APPEND_ALIGNED_ELEMENT : alignedByteOffset;
	DXGI_FORMAT format = DXGI_FORMAT_R32G32_FLOAT;;
	auto convertedInputClass = (D3D11_INPUT_CLASSIFICATION)convertInputClassification(inputSlotClass);
	m_ied.push_back({ semanticName, semanticIndex, format, inputSlot, (UINT)alignedByteOffset, convertedInputClass, instanceDataStepRate });

	UINT typeSize = sizeof(glm::vec2);
	if (convertedInputClass == D3D11_INPUT_PER_INSTANCE_DATA)
		InstanceSize += typeSize;
	else
		VertexSize += typeSize;

	InputLayout::pushVec2(inputType, semanticName, semanticIndex, inputSlotClass, instanceDataStepRate);
}
void DX11InputLayout::pushVec3(InputType inputType, const char* semanticName, unsigned int semanticIndex, unsigned int inputSlot, int alignedByteOffset, InputClassification inputSlotClass, unsigned int instanceDataStepRate) {
	if (m_ied.size() == m_ied.capacity()) {
		m_ied.reserve(m_ied.size() + 1);
		InputOrder.reserve(InputOrder.size() + 1);
	}
	alignedByteOffset = (alignedByteOffset == -1) ? (m_ied.size() == 0) ? 0 : D3D11_APPEND_ALIGNED_ELEMENT : alignedByteOffset;
	DXGI_FORMAT format = DXGI_FORMAT_R32G32B32_FLOAT;;
	auto convertedInputClass = (D3D11_INPUT_CLASSIFICATION)convertInputClassification(inputSlotClass);
	m_ied.push_back({ semanticName, semanticIndex, format, inputSlot, (UINT)alignedByteOffset, convertedInputClass, instanceDataStepRate });

	UINT typeSize = sizeof(glm::vec3);
	if (convertedInputClass == D3D11_INPUT_PER_INSTANCE_DATA)
		InstanceSize += typeSize;
	else
		VertexSize += typeSize;

	InputLayout::pushVec3(inputType, semanticName, semanticIndex, inputSlotClass, instanceDataStepRate);
}
void DX11InputLayout::pushVec4(InputType inputType, const char* semanticName, unsigned int semanticIndex, unsigned int inputSlot, int alignedByteOffset, InputClassification inputSlotClass, unsigned int instanceDataStepRate) {
	if (m_ied.size() == m_ied.capacity()) {
		m_ied.reserve(m_ied.size() + 1);
		InputOrder.reserve(InputOrder.size() + 1);
	}
	alignedByteOffset = (alignedByteOffset == -1) ? (m_ied.size() == 0) ? 0 : D3D11_APPEND_ALIGNED_ELEMENT : alignedByteOffset;
	DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT;;
	auto convertedInputClass = (D3D11_INPUT_CLASSIFICATION)convertInputClassification(inputSlotClass);
	m_ied.push_back({ semanticName, semanticIndex, format, inputSlot, (UINT)alignedByteOffset, convertedInputClass, instanceDataStepRate });

	UINT typeSize = sizeof(glm::vec4);
	if (convertedInputClass == D3D11_INPUT_PER_INSTANCE_DATA)
		InstanceSize += typeSize;
	else
		VertexSize += typeSize;

	InputLayout::pushVec4(inputType, semanticName, semanticIndex, inputSlotClass, instanceDataStepRate);
}

void DX11InputLayout::create(void* vertexShaderBlob) {
	if (!vertexShaderBlob) {
		Logger::Error("Failed to set up input layout, the ShaderSet does not have a vertex shader!");
		return;
	}
	auto shaderBlob = static_cast<ID3D10Blob*>(vertexShaderBlob);
	Application::getInstance()->getAPI<DX11API>()->getDevice()->CreateInputLayout(&m_ied[0], m_ied.size(), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), &m_inputLayout);
}

void DX11InputLayout::bind() const {
	Application::getInstance()->getAPI<DX11API>()->getDeviceContext()->IASetInputLayout(m_inputLayout);
}

int DX11InputLayout::convertInputClassification(InputClassification inputSlotClass) {
	switch (inputSlotClass) {
	case InputLayout::PER_VERTEX_DATA:
		return D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
	case InputLayout::PER_INSTANCE_DATA:
		return D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_INSTANCE_DATA;
	default:
		Logger::Error("Invalid input classifier specified");
		return 0;
	}
}
