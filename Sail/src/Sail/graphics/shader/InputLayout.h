#pragma once

#include <d3d11.h>
#include <vector>

class InputLayout {
public:
	enum InputType {
		POSITION,
		TEXCOORD,
		NORMAL,
		TANGENT,
		BITANGENT
	};
	
public:
	InputLayout();
	~InputLayout();

	template <typename T>
	void push(InputType inputType, LPCSTR semanticName, UINT semanticIndex, D3D11_INPUT_CLASSIFICATION inputSlotClass = D3D11_INPUT_PER_VERTEX_DATA, UINT instanceDataStepRate = 0);
	void create(ID3D10Blob* vertexShaderBlob);
	void bind() const;
	const std::vector<InputType>& getOrderedInputs() const;
	UINT getVertexSize() const;
	UINT getInstanceSize() const;

private:
	std::vector<D3D11_INPUT_ELEMENT_DESC> m_ied;
	ID3D11InputLayout* m_inputLayout;
	std::vector<InputType> m_inputOrder;
	UINT m_vertexSize;
	UINT m_instanceSize;

};

template <typename T>
void InputLayout::push(InputType inputType, LPCSTR semanticName, UINT semanticIndex, D3D11_INPUT_CLASSIFICATION inputSlotClass, UINT instanceDataStepRate) {
	if (m_ied.size() == m_ied.capacity()) {
		m_ied.reserve(m_ied.size() + 1);
		m_inputOrder.reserve(m_inputOrder.size() + 1);
	}
	UINT alignedByteOffset = (m_ied.size() == 0) ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;
	DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
	if (typeid(T) == typeid(glm::vec4)) { format = DXGI_FORMAT_R32G32B32A32_FLOAT; }
	if (typeid(T) == typeid(glm::vec3)) { format = DXGI_FORMAT_R32G32B32_FLOAT; }
	if (typeid(T) == typeid(glm::vec2)) { format = DXGI_FORMAT_R32G32_FLOAT; }
	if (typeid(T) == typeid(float)) { format = DXGI_FORMAT_R32_FLOAT; }
	m_ied.push_back({ semanticName, semanticIndex, format, 0, alignedByteOffset, inputSlotClass, instanceDataStepRate });

	UINT typeSize = sizeof(T);
	if (inputSlotClass == D3D11_INPUT_PER_INSTANCE_DATA)
		m_instanceSize += typeSize;
	else
		m_vertexSize += typeSize;

	m_inputOrder.push_back(inputType);
}

