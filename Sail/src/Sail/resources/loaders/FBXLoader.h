#pragma once

#include <fbxsdk.h>
#include <string>
#include "../../graphics/geometry/Model.h"

class FBXLoader {
public:
	FBXLoader(const std::string& filepath, ShaderPipeline* shaderSet);
	~FBXLoader();

	std::unique_ptr<Model>& getModel();

private:
	FbxScene* parseFBX(const std::string& filename);

	void loadNode(FbxNode* pNode);
	FbxVector2 getTexCoord(int cpIndex, FbxGeometryElementUV* geUV, FbxMesh* mesh, int polyIndex, int vertIndex) const;
	void getGeometry(FbxMesh* mesh, Mesh::Data& buildData);
	void getMaterial(FbxNode* pNode, Material* material);

private:
	static FbxManager* s_manager;
	static FbxIOSettings* s_ios;
	FbxScene* m_scene;

	std::string m_filepath;
	ShaderPipeline* m_shaderSet;
	std::unique_ptr<Model> m_model;
	//std::vector<Mesh::Data> m_meshDataList;

	/*Material::PhongSettings m_matSettings;
	std::string m_matDiffuseTex;
	std::string m_matSpecularTex;
	std::string m_matNormalTex;*/

};