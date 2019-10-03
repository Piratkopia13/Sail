#pragma once

#include <fbxsdk.h>
#include <string>
#include "../../graphics/geometry/Model.h"
#include <map>

class AnimationStack;

class FBXLoader {
public:
	FBXLoader();
	~FBXLoader();

	bool importScene(const std::string& filePath, Shader* path);
	bool initScene(const std::string& filePath);
	FbxScene* makeScene(std::string fileName, std::string sceneName);
	void clearScene(const std::string& filePath);
	void clearAllScenes();

	Model* importStaticModel(const std::string& filePath, Shader* shader);

	AnimationStack* importAnimationStack(const std::string& filePath);

	Model* fetchModel(const std::string& filePath, Shader* shader);
	AnimationStack* fetchAnimationStack(const std::string& filePath, Shader* shader = nullptr);

private:
	void fetchGeometry(FbxNode* node, Mesh::Data& mesh, const std::string& name);
	void getGeometry(FbxMesh* mesh, Mesh::Data& buildData, const std::string& scene);
	FbxVector2 getTexCoord(int cpIndex, FbxGeometryElementUV* geUV, FbxMesh* mesh, int polyIndex, int vertIndex) const;
	void getMaterial(FbxNode* pNode, Material* material);
	
	void fetchAnimations(FbxNode* node, AnimationStack* stack, const std::string& name);
	void getAnimations(FbxNode* node, AnimationStack* stack, const std::string& name);

	void addVertex(Mesh::Data& buildData, unsigned int& uniqueVertices, const unsigned long& currentIndex, const Mesh::vec3& position, const Mesh::vec3& normal, const Mesh::vec3& tangent, const Mesh::vec3& bitangent, const Mesh::vec2& uv);
	void fetchSkeleton(FbxNode* node, const std::string& filename);
	void fetchSkeletonRecursive(FbxNode* inNode, const std::string& filename, int inDepth, int myIndex, int inParentIndex);
	int getBoneIndex(unsigned int uniqueID, const std::string& name);

	//DEBUG
	std::string GetAttributeTypeName(FbxNodeAttribute::EType type);
	std::string PrintAttribute(FbxNodeAttribute* pAttribute);
	void printNodeTree(FbxNode* node, const std::string& indent);
	void printAnimationStack(const FbxNode* node);

private:
	static FbxManager* s_manager;
	static FbxIOSettings* s_ios;

	struct SceneData {
		~SceneData() { Memory::SafeDelete(model); Memory::SafeDelete(stack); }
		class Bone {
		public:
			Bone() { parentIndex = 0; uniqueID = 0; globalBindposeInverse = glm::identity<glm::mat4>(); };
			~Bone() {}

			int parentIndex;
			unsigned long uniqueID;
			std::vector<unsigned int> childIndexes;
			glm::mat4 globalBindposeInverse;
		};
		bool done;
		bool hasModel;
		bool hasAnimation;
		bool hasTextures;
		Model* model;
		AnimationStack* stack;

		std::vector<std::vector<unsigned long>> cpToVertMap;
		std::vector<FBXLoader::SceneData::Bone> bones;
	};

	std::map<std::string, const FbxScene*> m_scenes;
	std::map < std::string, SceneData> m_sceneData;
};