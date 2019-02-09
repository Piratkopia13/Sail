#pragma once

#include <SimpleMath.h>
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
		DirectX::SimpleMath::Vector3* positions;
		DirectX::SimpleMath::Vector3* normals;
		DirectX::SimpleMath::Vector4* colors;
		DirectX::SimpleMath::Vector2* texCoords;
		DirectX::SimpleMath::Vector3* tangents;
		DirectX::SimpleMath::Vector3* bitangents;
	};

public:
	Mesh(Data& buildData, ShaderSet* shaderSet);
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