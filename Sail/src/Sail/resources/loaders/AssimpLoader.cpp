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

	if (!scene) {
		return nullptr;
	}
	
	std::string name = scene->GetShortFilename(path.c_str());
	
	


	for (int i = 0; i < scene->mRootNode->mNumChildren; i++) {
		//scene->mRootNode->mChildren[i]->


	}
	




	if (scene->HasMeshes()) {
		//importMesh(scene->mRootNode->mChildren[i], scene);
	}
		
	if (scene->HasAnimations()) {
		for (int i = 0; i < scene->mNumAnimations; i++) {
			std::string name = scene->mAnimations[i]->mName.C_Str();
			Logger::Log(name);

		}




	}

	

	



	return nullptr;
}

Model* AssimpLoader::importAnimatedModel(const std::string& path, Shader* shader) {
	return nullptr;
}

std::vector<Model*> AssimpLoader::importScene(const std::string& path, Shader* shader) {
	return std::vector<Model*>();
}

Mesh* AssimpLoader::importMesh(const aiScene* scene) {
	//if()
	//Mesh::Data data;
	//data.positions = new Mesh::vec3[3030];



	return nullptr;
}

Animation* AssimpLoader::importAnimation(const aiScene* scene) {
	


	
	return nullptr;
}

const bool AssimpLoader::errorCheck(const aiScene* scene) {
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		Logger::Error("ERROR::ASSIMP::" + std::string(m_importer.GetErrorString()));
		return true;
	}
	return false;
}
