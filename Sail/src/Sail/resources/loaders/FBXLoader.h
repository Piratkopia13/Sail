#pragma once

#include <fbxsdk.h>
#include <string>
#include "../../graphics/geometry/Model.h"
#include <map>

class AnimationStack;

class FBXLoader {
public:
	FBXLoader(const std::string& filepath, Shader* shader);
	FBXLoader();
	~FBXLoader();

	bool initScene(const std::string& filePath);
	bool importScene(const std::string& filePath);
	void clearScene(const std::string& filePath);
	void clearAllScenes();
	Model* importStaticModel(const std::string& filePath, Shader* path);
	AnimationStack* importStack(const std::string& filePath);



	FbxScene* makeScene(std::string fileName, std::string sceneName);
	



	std::unique_ptr<Model>& getModel();

private:


	void fetchGeometry(const FbxNode* node, Mesh* mesh);



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
	Shader* m_shader;
	std::unique_ptr<Model> m_model;
	//std::vector<Mesh::Data> m_meshDataList;

	/*Material::PhongSettings m_matSettings;
	std::string m_matDiffuseTex;
	std::string m_matSpecularTex;
	std::string m_matNormalTex;*/




	struct SceneData {
		std::vector<Model*> models;
		std::vector<AnimationStack*> animationStacks;
		std::vector<std::string> textures;

	};

	std::map<std::string, const FbxScene*> m_scenes;
	std::map < std::string, SceneData> m_sceneData;

	//DEBUG
	std::string GetAttributeTypeName(FbxNodeAttribute::EType type);
	std::string PrintAttribute(FbxNodeAttribute* pAttribute);
	void printAnimationStack(const FbxNode* node);
	//void printAnimationStack();




};