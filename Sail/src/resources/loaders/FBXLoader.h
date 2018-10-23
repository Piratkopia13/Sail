#pragma once

#include <d3d11.h>
#include <SimpleMath.h>
#include <fbxsdk.h>
#include <string>
#include "../../graphics/geometry/Model.h"

class FBXLoader {
public:
	FBXLoader(const std::string& filepath, ShaderSet* shaderSet);
	~FBXLoader();

	std::unique_ptr<Model>& getModel();

private:
	FbxScene* parseFBX(const std::string& filename);

	void loadNode(FbxNode* pNode);
	FbxVector2 getTexCoord(int cpIndex, FbxGeometryElementUV* geUV, FbxMesh* mesh, int polyIndex, int vertIndex) const;
	void getGeometry(FbxMesh* mesh, Mesh::Data& buildData);
	void getMaterial(FbxNode* pNode, Material* material);

private:
	struct VertexData {
		DirectX::SimpleMath::Vector3 position;
		DirectX::SimpleMath::Vector3 normal;
		DirectX::SimpleMath::Vector3 tangent;
		DirectX::SimpleMath::Vector3 bitangent;
		DirectX::SimpleMath::Vector2 texCoord;
	};

	bool findIndex(const Mesh::Data& buildData, unsigned int numVertices, const VertexData& vertexData, unsigned int& foundIndex);

private:
	static FbxManager* s_manager;
	static FbxIOSettings* s_ios;
	FbxScene* m_scene;

	std::string m_filepath;
	ShaderSet* m_shaderSet;
	std::unique_ptr<Model> m_model;
	//std::vector<Mesh::Data> m_meshDataList;

	/*Material::PhongSettings m_matSettings;
	std::string m_matDiffuseTex;
	std::string m_matSpecularTex;
	std::string m_matNormalTex;*/

};