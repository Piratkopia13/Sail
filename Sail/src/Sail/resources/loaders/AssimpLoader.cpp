#include "pch.h"
#include "AssimpLoader.h"

AssimpLoader::AssimpLoader() :
m_importer()
{

}

AssimpLoader::~AssimpLoader() {
	//delete m_importer;
}

Model* AssimpLoader::importModel(const std::string& path, Shader* shader) {
	const aiScene* scene = m_importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
	if (errorCheck(scene)) {
		return nullptr;
	}
	std::string name = scene->GetShortFilename(path.c_str());
	//processNode(scene, scene->mRootNode);

	return nullptr;
}

AnimationStack* AssimpLoader::importAnimationStack(const std::string& path) {
	const aiScene* scene = m_importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
	if (errorCheck(scene)) {
		return nullptr;
	}
	std::string name = scene->GetShortFilename(path.c_str());
	//processNode(scene, scene->mRootNode);

	size_t vertCount = 0;
	
	makeOffsets(scene);
	for (size_t i = 0; i < scene->mNumMeshes; i++) {
		vertCount += scene->mMeshes[i]->mNumVertices;
	}
	AnimationStack* stack = new AnimationStack(vertCount);

	std::map<std::string, size_t> boneMap;
	if (!importBonesFromNode(scene, scene->mRootNode, stack, boneMap)) {
		Memory::SafeDelete(stack);
		return nullptr;
	}
	return stack;
}

std::vector<Model*> AssimpLoader::importScene(const std::string& path, Shader* shader) {
	return std::vector<Model*>();
}




Mesh* AssimpLoader::importMesh(const aiScene* scene, aiNode* node) {
	return nullptr;
}

bool AssimpLoader::importBonesFromNode(const aiScene* scene, aiNode* node, AnimationStack* stack, std::map<std::string, size_t>& map) {
	Animation* animation = new Animation();


	#ifdef _DEBUG 
	Logger::Log("1"+std::string(node->mName.C_Str())); 
	#endif
	for (size_t nodeID = 0; nodeID < node->mNumMeshes; nodeID++) {
		const aiMesh* mesh = scene->mMeshes[node->mMeshes[nodeID]];
		
			#ifdef _DEBUG 
		Logger::Log("2 "+std::string(mesh->mName.C_Str()));
			#endif
		if (mesh->HasBones()) {
			for (size_t boneID = 0; boneID < mesh->mNumBones; boneID++) {
				const aiBone* bone = mesh->mBones[boneID];
				
				std::string boneName = bone->mName.C_Str();
				size_t index = stack->m_boneMap.size();
				if (map.find(boneName) == map.end()) {
					stack->m_boneMap[boneName] = { index, mat4_cast(bone->mOffsetMatrix) };

				}
				else {
					index = stack->m_boneMap[boneName].index;
				}
				


				#ifdef _DEBUG 
				Logger::Log("3  "+std::string(bone->mName.C_Str()));
				#endif



				for (size_t weightID = 0; weightID < bone->mNumWeights; weightID++) {
					const aiVertexWeight weight = bone->mWeights[weightID];
					stack->setConnectionData(m_meshOffsets[node->mMeshes[nodeID]]+weight.mVertexId, index, weight.mWeight);

				}


			}

		}
		else {
			Logger::Log("3  No Bones");
		}

	}




	int size = node->mNumChildren;
	for (size_t i = 0; i < size; i++) {
		importBonesFromNode(scene, node->mChildren[i], stack, map);
	}


	return true;
}

bool AssimpLoader::importAnimations(const aiScene* scene, AnimationStack* stack) {





	return false;
}

//Animation* AssimpLoader::importAnimation(const aiScene* scene, aiNode* node) {
//
//	if (scene->HasAnimations()) {
//		for (int i = 0; i < scene->mNumAnimations; i++) {
//			std::string name = scene->mAnimations[i]->mName.C_Str();
//			Logger::Log(name);
//
//		}
//
//
//
//
//	}
//	return ;
//}

const bool AssimpLoader::errorCheck(const aiScene* scene) {
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		Logger::Error("ERROR::ASSIMP::" + std::string(m_importer.GetErrorString()));
		return true;
	}
	return false;
}

