#include "pch.h"
#include "Mesh.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Sail/graphics/shader/ShaderPipeline.h"
#include "Sail/Application.h"

Mesh::Mesh(Data& buildData, ShaderPipeline* shaderSet)
	: meshData(buildData) 
{
	
}

Mesh::~Mesh() {
	Memory::safeDeleteArr(meshData.indices);
	Memory::safeDeleteArr(meshData.positions);
	Memory::safeDeleteArr(meshData.normals);
	Memory::safeDeleteArr(meshData.bitangents);
	Memory::safeDeleteArr(meshData.colors);
	Memory::safeDeleteArr(meshData.tangents);
	Memory::safeDeleteArr(meshData.texCoords);
}

Material* Mesh::getMaterial() {
	return material.get();
}

UINT Mesh::getNumVertices() const {
	return meshData.numVertices;
}
UINT Mesh::getNumIndices() const {
	return meshData.numIndices;
}
UINT Mesh::getNumInstances() const {
	return meshData.numInstances;
}
const VertexBuffer& Mesh::getVertexBuffer() const {
	return *vertexBuffer;
}
const IndexBuffer& Mesh::getIndexBuffer() const {
	return *indexBuffer;
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
		this->positions = new glm::vec3[numVerts];
		for (UINT i = 0; i < numVerts; i++)
			this->positions[i] = other.positions[i];
	}
	if (other.normals) {
		this->normals = new glm::vec3[numVerts];
		for (UINT i = 0; i < numVerts; i++)
			this->normals[i] = other.normals[i];
	}
	if (other.colors) {
		this->colors = new glm::vec4[numVerts];
		for (UINT i = 0; i < numVerts; i++)
			this->colors[i] = other.colors[i];
	}
	if (other.texCoords) {
		this->texCoords = new glm::vec2[numVerts];
		for (UINT i = 0; i < numVerts; i++)
			this->texCoords[i] = other.texCoords[i];
	}
	if (other.tangents) {
		this->tangents = new glm::vec3[numVerts];
		for (UINT i = 0; i < numVerts; i++)
			this->tangents[i] = other.tangents[i];
	}
	if (other.bitangents) {
		this->bitangents = new glm::vec3[numVerts];
		for (UINT i = 0; i < numVerts; i++)
			this->bitangents[i] = other.bitangents[i];
	}
}