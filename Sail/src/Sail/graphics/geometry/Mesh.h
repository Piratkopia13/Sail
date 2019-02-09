#pragma once

#include <d3d11.h>
#include <SimpleMath.h>
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
		DirectX::SimpleMath::Vector3* positions;
		DirectX::SimpleMath::Vector3* normals;
		DirectX::SimpleMath::Vector4* colors;
		DirectX::SimpleMath::Vector2* texCoords;
		DirectX::SimpleMath::Vector3* tangents;
		DirectX::SimpleMath::Vector3* bitangents;
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

