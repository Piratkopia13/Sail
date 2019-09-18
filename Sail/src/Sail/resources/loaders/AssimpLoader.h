#pragma once
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "../../graphics/geometry/Model.h"
#include "../../graphics/geometry/Animation.h"
#include <map>


class AssimpLoader {
public:
	AssimpLoader();
	~AssimpLoader();

	Model* importModel(const std::string& path, Shader* shader);
	AnimationStack* importAnimationStack(const std::string& path);
	std::vector<Model*> importScene(const std::string& path, Shader* shader);

	



private:

	
	Mesh* importMesh(const aiScene* scene, aiNode* node);
	bool importBonesFromNode(const aiScene* scene, aiNode* node, AnimationStack* stack);
	bool importAnimations(const aiScene* scene, AnimationStack* stack);
	void readNodeHierarchy(const unsigned int animationID, const unsigned int frame, const float animationTime, const aiNode* node, const glm::mat4& parent, Animation::Frame* animationFrame);

	const bool errorCheck(const aiScene* scene);
	void clearData();
	//static inline glm::mat4 mat4_cast(const aiMatrix4x4& m) { 
	//	return glm::transpose(glm::make_mat4(&m.a1)); 
	//}
	glm::mat4 mat4_cast(const aiMatrix4x4& aiMat) {
		return {
		aiMat.a1, aiMat.b1, aiMat.c1, aiMat.d1,
		aiMat.a2, aiMat.b2, aiMat.c2, aiMat.d2,
		aiMat.a3, aiMat.b3, aiMat.c3, aiMat.d3,
		aiMat.a4, aiMat.b4, aiMat.c4, aiMat.d4
		};
	}

	void makeOffsets(const aiScene* scene) {
		m_meshOffsets.emplace_back(0);
		unsigned int old = 0;
		if (scene->mNumMeshes > 1) {
			for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
				m_meshOffsets.emplace_back(old + scene->mMeshes[i]->mNumVertices);
				old = m_meshOffsets.back();
			}
		}
	}

	struct BoneInfo {
		unsigned int index;
		std::string nodeName;
		glm::mat4 offset;
	};
	std::unordered_map<std::string, const aiNode*> m_nodes;
	std::unordered_map<std::string, AssimpLoader::BoneInfo> m_boneMap;
	std::vector<int> m_meshOffsets;
	Assimp::Importer m_importer;
	std::vector<std::map<std::string, const aiNodeAnim*>> m_channels;
	glm::mat4 m_globalTransform;
	void mapChannels(const aiScene* scene) {
		m_channels.resize(scene->mNumAnimations);
		for (unsigned int i = 0; i < scene->mNumAnimations; i++) {
			const aiAnimation* animation = scene->mAnimations[i];

			for (unsigned int channel = 0; channel < animation->mNumChannels; channel++) {
				m_channels[i][animation->mChannels[channel]->mNodeName.C_Str()] = animation->mChannels[channel];
			}
		}
	}
};