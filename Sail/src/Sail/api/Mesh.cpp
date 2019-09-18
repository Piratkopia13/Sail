#include "pch.h"
#include "Mesh.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Sail/api/shader/ShaderPipeline.h"
#include "Sail/Application.h"

Mesh::Mesh(Data& buildData, Shader* shader)
{
	meshData.deepCopy(buildData);
}




Mesh::~Mesh() {
	Memory::SafeDeleteArr(meshData.indices);
	Memory::SafeDeleteArr(meshData.positions);
	Memory::SafeDeleteArr(meshData.normals);
	Memory::SafeDeleteArr(meshData.bitangents);
	Memory::SafeDeleteArr(meshData.colors);
	Memory::SafeDeleteArr(meshData.tangents);
	Memory::SafeDeleteArr(meshData.texCoords);
}



const Mesh::Data& Mesh::getMeshData() {
	return meshData;
}

Material* Mesh::getMaterial() {
	return material.get();
}

unsigned int Mesh::getNumVertices() const {
	return meshData.numVertices;
}
unsigned int Mesh::getNumIndices() const {
	return meshData.numIndices;
}
unsigned int Mesh::getNumInstances() const {
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
		this->indices = SAIL_NEW unsigned long[other.numIndices];
		for (unsigned int i = 0; i < other.numIndices; i++)
			this->indices[i] = other.indices[i];
	}
	unsigned int numVerts = (other.numIndices > 0) ? other.numVertices : other.numIndices;
	if (other.positions) {
		this->positions = SAIL_NEW Mesh::vec3[numVerts];
		for (unsigned int i = 0; i < numVerts; i++)
			this->positions[i] = other.positions[i];
	}
	if (other.normals) {
		this->normals = SAIL_NEW Mesh::vec3[numVerts];
		for (unsigned int i = 0; i < numVerts; i++)
			this->normals[i] = other.normals[i];
	}
	if (other.colors) {
		this->colors = SAIL_NEW Mesh::vec4[numVerts];
		for (unsigned int i = 0; i < numVerts; i++)
			this->colors[i] = other.colors[i];
	}
	if (other.texCoords) {
		this->texCoords = SAIL_NEW Mesh::vec2[numVerts];
		for (unsigned int i = 0; i < numVerts; i++)
			this->texCoords[i] = other.texCoords[i];
	}
	if (other.tangents) {
		this->tangents = SAIL_NEW Mesh::vec3[numVerts];
		for (unsigned int i = 0; i < numVerts; i++)
			this->tangents[i] = other.tangents[i];
	}
	if (other.bitangents) {
		this->bitangents = SAIL_NEW Mesh::vec3[numVerts];
		for (unsigned int i = 0; i < numVerts; i++)
			this->bitangents[i] = other.bitangents[i];
	}
}