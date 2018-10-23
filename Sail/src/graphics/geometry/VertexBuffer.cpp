#include "VertexBuffer.h"
#include "../../api/Application.h"
#include "../../utils/Utils.h"

using namespace DirectX::SimpleMath;

VertexBuffer::VertexBuffer(const InputLayout& inputLayout, Mesh::Data& modelData)
	: m_instanceBuffer(nullptr)
	, m_inputLayout(inputLayout)
{

	m_stride = inputLayout.getVertexSize();
	
	if (m_stride == 0) {
		Logger::Error("Input layout not set up properly for shader");
		__debugbreak();
	}

	void* vertices = malloc(modelData.numVertices * m_stride);

	UINT byteOffset = 0;
	for (UINT i = 0; i < modelData.numVertices; i++) {

		// Loop through the input layout to get the order the data should have
		for (const InputLayout::InputType& inputType : inputLayout.getOrderedInputs()) {
			void* addr = (char*)vertices + byteOffset;

			if (inputType == InputLayout::POSITION) {
				UINT size = sizeof(Vector3);
				memcpy(addr, &modelData.positions[i], size);
				byteOffset += size;
			}
			else if (inputType == InputLayout::TEXCOORD) {
				UINT size = sizeof(Vector2);
				// Check if model data contains texCoords
				if (modelData.texCoords)
					memcpy(addr, &modelData.texCoords[i], size);
				else 
					memset(addr, 0, size);
				byteOffset += size;
			} 
			else if (inputType == InputLayout::NORMAL) {
				UINT size = sizeof(Vector3);
				if (modelData.normals)
					memcpy(addr, &modelData.normals[i], size);
				else
					memset(addr, 0, size);
				byteOffset += size;
			}
			else if (inputType == InputLayout::TANGENT) {
				UINT size = sizeof(Vector3);
				if (modelData.tangents)
					memcpy(addr, &modelData.tangents[i], size);
				else
					memset(addr, 0, size);
				byteOffset += size;
			}
			else if (inputType == InputLayout::BITANGENT) {
				UINT size = sizeof(Vector3);
				if (modelData.bitangents)
					memcpy(addr, &modelData.bitangents[i], size);
				else
					memset(addr, 0, size);
				byteOffset += size;
			}

		}

	}

	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));

	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	//vbd.ByteWidth = modelData.numVertices * sizeof(DeferredGeometryShader::Vertex);
	vbd.ByteWidth = modelData.numVertices * m_stride;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexData;
	ZeroMemory(&vertexData, sizeof(vertexData));
	vertexData.pSysMem = vertices;

	// Create the vertex buffer
	ThrowIfFailed(Application::getInstance()->getAPI()->getDevice()->CreateBuffer(&vbd, &vertexData, &m_vertBuffer));
	// Delete vertices from cpu memory
	free(vertices);


	// Set up instanceData buffer if instances are set
	if (modelData.numInstances > 0) {
		// Set up instance buffer
		D3D11_BUFFER_DESC ibd;
		ZeroMemory(&ibd, sizeof(ibd));
		ibd.Usage = D3D11_USAGE_DYNAMIC;
		ibd.ByteWidth = inputLayout.getInstanceSize() * modelData.numInstances;
		ibd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		ibd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		ibd.MiscFlags = 0;
		ibd.StructureByteStride = 0;

		ThrowIfFailed(Application::getInstance()->getAPI()->getDevice()->CreateBuffer(&ibd, nullptr, &m_instanceBuffer));
	}

}


VertexBuffer::~VertexBuffer() {
	Memory::safeRelease(m_vertBuffer);
	Memory::safeRelease(m_instanceBuffer);
}

ID3D11Buffer* const* VertexBuffer::getBuffer() const {
	return &m_vertBuffer;
}

void VertexBuffer::bind() const {
	if (m_instanceBuffer) {
		// Bind both vertex and instance buffers
		UINT strides[2] = { m_inputLayout.getVertexSize(), m_inputLayout.getInstanceSize() };
		UINT offsets[2] = { 0, 0 };
		ID3D11Buffer* bufferPtrs[2] = { m_vertBuffer, m_instanceBuffer };
		Application::getInstance()->getAPI()->getDeviceContext()->IASetVertexBuffers(0, 2, bufferPtrs, strides, offsets);
	} else {
		// Bind vertex buffer
		UINT offset = 0;
		Application::getInstance()->getAPI()->getDeviceContext()->IASetVertexBuffers(0, 1, &m_vertBuffer, &m_stride, &offset);
	}
}
