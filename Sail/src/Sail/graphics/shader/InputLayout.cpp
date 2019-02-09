#include "pch.h"
#include "InputLayout.h"
#include "Sail/Application.h"

InputLayout::InputLayout() {
	m_vertexSize = 0;
}

InputLayout::~InputLayout() {
	Memory::safeRelease(m_inputLayout);
}

void InputLayout::create(ID3D10Blob* vertexShaderBlob) {
	if (!vertexShaderBlob)
		Logger::Error("Failed to set up input layout, the ShaderSet does not have a vertex shader!");
	Application::getInstance()->getAPI()->getDevice()->CreateInputLayout(&m_ied[0], m_ied.size(), vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), &m_inputLayout);
}

void InputLayout::bind() const {
	Application::getInstance()->getAPI()->getDeviceContext()->IASetInputLayout(m_inputLayout);
}

const std::vector<InputLayout::InputType>& InputLayout::getOrderedInputs() const {
	return m_inputOrder;
}

UINT InputLayout::getVertexSize() const {
	return m_vertexSize;
}

UINT InputLayout::getInstanceSize() const {
	return m_instanceSize;
}
