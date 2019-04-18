#pragma once

#include <glm/glm.hpp>
#include <memory>
#include "Sail/graphics/geometry/Material.h"
#include "Sail/graphics/renderer/Renderer.h"

class VertexBuffer;
class IndexBuffer;

class Mesh {
public:
	typedef std::unique_ptr<Mesh> Ptr;
	typedef std::shared_ptr<Mesh> SPtr;

	struct Data {
		Data() : numIndices(0), numInstances(0), indices(nullptr), numVertices(0), normals(nullptr), positions(nullptr), colors(nullptr), texCoords(nullptr), tangents(nullptr), bitangents(nullptr) {};
		void deepCopy(const Data& other);
		UINT numIndices;
		ULONG* indices;
		UINT numVertices;
		UINT numInstances;
		glm::vec3* positions;
		glm::vec3* normals;
		glm::vec4* colors;
		glm::vec2* texCoords;
		glm::vec3* tangents;
		glm::vec3* bitangents;
	};

public:
	static Mesh* Create(Data& buildData, ShaderPipeline* shaderSet);
	Mesh(Data& buildData, ShaderPipeline* shaderSet);
	virtual ~Mesh();

	virtual void draw(const Renderer& renderer) = 0;

	Material* getMaterial();

	UINT getNumVertices() const;
	UINT getNumIndices() const;
	UINT getNumInstances() const;
	const VertexBuffer& getVertexBuffer() const;
	const IndexBuffer& getIndexBuffer() const;

protected:
	Material::SPtr material;

	std::unique_ptr<VertexBuffer> vertexBuffer;
	std::unique_ptr<IndexBuffer> indexBuffer;
	Data meshData;

};