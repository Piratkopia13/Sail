#include "pch.h"
#include "Mesh.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "../shader/ShaderSet.h"
#include "Sail/Application.h"

Mesh::Mesh(Data& buildData, ShaderSet* shaderSet) 
	: m_data(buildData)
{
	m_material = std::make_shared<Material>(shaderSet);
	// Create vertex buffer
	m_vertexBuffer = std::make_unique<VertexBuffer>(shaderSet->getInputLayout(), buildData);
	// Create index buffer is indices are set
	if (buildData.numIndices > 0) {
		m_indexBuffer = std::make_unique<IndexBuffer>(buildData);
	}
}

Mesh::~Mesh() {
	Memory::safeDeleteArr(m_data.indices);
	Memory::safeDeleteArr(m_data.positions);
	Memory::safeDeleteArr(m_data.normals);
	Memory::safeDeleteArr(m_data.bitangents);
	Memory::safeDeleteArr(m_data.colors);
	Memory::safeDeleteArr(m_data.tangents);
	Memory::safeDeleteArr(m_data.texCoords);
}

void Mesh::draw(const Renderer& renderer) {
	m_material->bind();

	m_vertexBuffer->bind();
	if (m_indexBuffer)
		m_indexBuffer->bind();

	// TODO: replace following DirectX calls with abstraction layer

	auto* devCon = Application::getInstance()->getAPI()->getDeviceContext();
	// Set topology
	devCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// Draw call
	if (m_indexBuffer)
		devCon->DrawIndexed(getNumIndices(), 0U, 0U);
	else
		devCon->Draw(getNumVertices(), 0);
}

Material* Mesh::getMaterial() {
	return m_material.get();
}

UINT Mesh::getNumVertices() const {
	return m_data.numVertices;
}
UINT Mesh::getNumIndices() const {
	return m_data.numIndices;
}
UINT Mesh::getNumInstances() const {
	return m_data.numInstances;
}
const VertexBuffer& Mesh::getVertexBuffer() const {
	return *m_vertexBuffer;
}
const IndexBuffer& Mesh::getIndexBuffer() const {
	return *m_indexBuffer;
}

void Mesh::Data::deepCopy(const Data& other) {
	this->numIndices = other.numIndices;
	this->numVertices = other.numVertices;
	this->numInstances = other.numInstances;
	if (other.indices) {
		this->indices = new ULONG[other.numIndices];
		for (UINT i = 0; i < other.numIndices; i++)
			this->indices[i] = other.indices[i];
	}
	UINT numVerts = (other.numIndices > 0) ? other.numIndices : other.numVertices;
	if (other.positions) {
		this->positions = new DirectX::SimpleMath::Vector3[numVerts];
		for (UINT i = 0; i < numVerts; i++)
			this->positions[i] = other.positions[i];
	}
	if (other.normals) {
		this->normals = new DirectX::SimpleMath::Vector3[numVerts];
		for (UINT i = 0; i < numVerts; i++)
			this->normals[i] = other.normals[i];
	}
	if (other.colors) {
		this->colors = new DirectX::SimpleMath::Vector4[numVerts];
		for (UINT i = 0; i < numVerts; i++)
			this->colors[i] = other.colors[i];
	}
	if (other.texCoords) {
		this->texCoords = new DirectX::SimpleMath::Vector2[numVerts];
		for (UINT i = 0; i < numVerts; i++)
			this->texCoords[i] = other.texCoords[i];
	}
	if (other.tangents) {
		this->tangents = new DirectX::SimpleMath::Vector3[numVerts];
		for (UINT i = 0; i < numVerts; i++)
			this->tangents[i] = other.tangents[i];
	}
	if (other.bitangents) {
		this->bitangents = new DirectX::SimpleMath::Vector3[numVerts];
		for (UINT i = 0; i < numVerts; i++)
			this->bitangents[i] = other.bitangents[i];
	}
}