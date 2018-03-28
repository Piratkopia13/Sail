#pragma once

#include <d3d11.h>
#include <SimpleMath.h>
#include <limits>
#include "../../utils/Utils.h"
#include "Transform.h"
#include "spatial/AABB.h"
#include "../renderer/Renderer.h"

// Forward declarations
class ShaderSet;
class Material;
class VertexBuffer;
class IndexBuffer;

class Model {
public: 

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

	Model(Data& buildData, ShaderSet* shaderSet);
	Model(const std::string& path, ShaderSet* shaderSet);
	~Model();

	//void setBuildData(Data& buildData);
	const Data& getBuildData() const;
	//void buildBufferForShader(ShaderSet* shader);

	// Draws the model using its material
	void draw(Renderer& renderer, bool bindShader = true);

	UINT getNumVertices() const;
	UINT getNumIndices() const;
	UINT getNumInstances() const;
	const VertexBuffer& getVertexBuffer() const;
	const IndexBuffer& getIndexBuffer() const;
	ID3D11Buffer* getInstanceBuffer() const;
	void setTransform(Transform* newTransform);
	Transform& getTransform();
	ShaderSet* getShader() const;
	Material* getMaterial();
	const AABB& getAABB() const;
	void updateAABB();

private:
	void calculateAABB();

private:
	std::unique_ptr<VertexBuffer> m_vertexBuffer;
	std::unique_ptr<IndexBuffer> m_indexBuffer;
	//ID3D11Buffer* m_instanceBuffer;

	bool m_transformChanged;
	Transform* m_transform;
	Material* m_material;

	Data m_data;

	AABB m_aabb;

};
