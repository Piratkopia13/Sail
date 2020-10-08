#pragma once

#include <fbxsdk.h>
#include <string>
#include "sail/api/Mesh.h"

class PhongMaterial;

class FBXLoader {
public:
	FBXLoader(const std::string& filepath);
	~FBXLoader();

	std::shared_ptr<Mesh>& getMesh();

private:
	FbxScene* parseFBX(const std::string& filename);

	void loadNode(FbxNode* pNode);
	FbxVector2 getTexCoord(int cpIndex, FbxGeometryElementUV* geUV, FbxMesh* mesh, int polyIndex, int vertIndex) const;
	void getGeometry(FbxMesh* mesh, Mesh::Data& buildData);
	void getMaterial(FbxNode* pNode, PhongMaterial* material);

private:
	static FbxManager* s_manager;
	static FbxIOSettings* s_ios;
	FbxScene* m_scene;

	std::string m_filepath;
	std::shared_ptr<Mesh> m_mesh;


};