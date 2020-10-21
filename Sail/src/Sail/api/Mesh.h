#pragma once

#include <glm/glm.hpp>
#include <memory>
#include "Sail/api/Renderer.h"
#include "Sail/graphics/material/Material.h"

class Shader;
class VertexBuffer;
class IndexBuffer;

class Mesh {
public:
	typedef std::unique_ptr<Mesh> Ptr;
	typedef std::shared_ptr<Mesh> SPtr;

	struct vec2 {
		glm::vec2 vec;
		vec2(float x = 0.f, float y = 0.f) {
			this->vec.x = x; this->vec.y = y;
		}
		const bool operator==(const vec2& other) const {
			return this->vec == other.vec;
		}
	};
	struct vec3 {
		glm::vec3 vec;
		vec3(float x = 0.f, float y = 0.f, float z = 0.f) {
			this->vec.x = x; this->vec.y = y; this->vec.z = z;
		}
		const bool operator==(const vec3& other) const{
			return this->vec == other.vec;
		}
	};
	struct vec4 {
		glm::vec4 vec;
		vec4(float x = 0.f, float y = 0.f, float z = 0.f, float w = 0.f) {
			this->vec.x = x; this->vec.y = y; this->vec.z = z; this->vec.w = w;
		}
		const bool operator==(const vec4& other) const {
			return this->vec == other.vec;
		}
	};

	struct Data {
		Data() : numIndices(0), numInstances(0), indices(nullptr), numVertices(0), normals(nullptr), positions(nullptr), colors(nullptr), texCoords(nullptr), tangents(nullptr), bitangents(nullptr) {};
		void deepCopy(const Data& other);
		//uint32_t calculateVertexStride() const;
		uint32_t numIndices;
		uint32_t* indices;
		uint32_t numVertices;
		uint32_t numInstances;
		Mesh::vec3* positions;
		Mesh::vec3* normals;
		Mesh::vec4* colors;
		Mesh::vec2* texCoords;
		Mesh::vec3* tangents;
		Mesh::vec3* bitangents;
	};

public:
	static Mesh* Create(Data& buildData);
	Mesh(Data& buildData);
	virtual ~Mesh();

	virtual void draw(const Renderer& renderer, Material* material, Shader* shader, void* cmdList = nullptr) = 0;

	// Returns a unique hash for the combination of vertex data used in the mesh (positions, normals, etc)
	uint32_t getAttributesHash();

	uint32_t getNumVertices() const;
	uint32_t getNumIndices() const;
	uint32_t getNumInstances() const;
	VertexBuffer& getVertexBuffer() const;
	IndexBuffer& getIndexBuffer() const;

protected:
	std::unique_ptr<VertexBuffer> vertexBuffer;
	std::unique_ptr<IndexBuffer> indexBuffer;
	Data meshData;

};