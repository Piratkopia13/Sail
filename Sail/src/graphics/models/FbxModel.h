#pragma once

#include <string>
#include <cstdio>

#include "../../resources/parsers/FBXParser.h"
#include "../geometry/Model.h"
#include "../../api/Application.h"
#include "../geometry/Material.h"
#include "../geometry/factory/CubeModel.h"


namespace {
	static const std::string DEFAULT_MODEL_LOCATION = "res/models/";
}
// TODO: Rename and call from Model(path&, shader) constructor
class FbxModel{
public:
	FbxModel(const std::string& filename);

	~FbxModel();

	Model* getModel();

private:
	void loadNode(FbxNode* pNode, const std::string& filename);	
	FbxVector2 getTexCoord(int cpIndex, FbxGeometryElementUV* geUV, FbxMesh* mesh, int polyIndex, int vertIndex) const;
	void fetchGeometry(FbxMesh* mesh, const std::string& filename);
	void fetchMaterial(FbxNode* pNode, const std::string& fbxFilename);

private:
	FbxScene* m_scene;
	std::unique_ptr<Model> m_model;
	Mesh::Data m_buildData;

	struct Bone {
		Bone* parent;
		DirectX::SimpleMath::Matrix LTMatrix,
			GTMatrix,
			invBindPose;
	};

	struct Skeleton {
		Bone* bones;
	};

};