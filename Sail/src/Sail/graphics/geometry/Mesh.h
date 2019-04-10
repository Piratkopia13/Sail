#pragma once

#include <d3d11.h>
#include <glm/glm.hpp>
#include <memory>
#include "Material.h"
#include "../renderer/Renderer.h"

class VertexBuffer;
class IndexBuffer;
class ShaderSet;

class MeshOld {
public:
	typedef std::unique_ptr<MeshOld> Ptr;
	typedef std::shared_ptr<MeshOld> SPtr;
public:
	struct Data {
		Data() : numIndices(0), numInstances(0), indices(nullptr), numVertices(0), normals(nullptr), positions(nullptr), colors(nullptr), texCoords(nullptr), tangents(nullptr), bitangents(nullptr) { };
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
	MeshOld(Data& buildData, ShaderSet* shaderSet);
	~MeshOld();

	static MeshOld* getFullscreenQuad();

	void draw(const Renderer& renderer);

	//const Data& getBuildData() const;
	Material* getMaterial();

	UINT getNumVertices() const;
	UINT getNumIndices() const;
	UINT getNumInstances() const;
	const VertexBuffer& getVertexBuffer() const;
	const IndexBuffer& getIndexBuffer() const;

private:
	Material::SPtr m_material;

	std::unique_ptr<VertexBuffer> m_vertexBuffer;
	std::unique_ptr<IndexBuffer> m_indexBuffer;
	Data m_data;

};

