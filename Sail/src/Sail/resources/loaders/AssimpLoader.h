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
	bool importBonesFromNode(const aiScene* scene, aiNode* node, AnimationStack* stack, std::map<std::string, size_t>& map);
	bool importAnimations(const aiScene* scene, AnimationStack* stack);
	const bool errorCheck(const aiScene* scene);

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
		if (scene->mNumMeshes >= 1) {
			m_meshOffsets.emplace_back(0);
			for (size_t i = 1; i < scene->mNumMeshes; i++) {
				m_meshOffsets.emplace_back(scene->mMeshes[i]->mNumVertices);
			}
		}
	}

	std::vector<int> m_meshOffsets;
	Assimp::Importer m_importer;

};