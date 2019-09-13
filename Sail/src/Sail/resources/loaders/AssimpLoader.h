#pragma once
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "../../graphics/geometry/Model.h"
#include "../../graphics/geometry/Animation.h"


class AssimpLoader {
public:
	AssimpLoader();
	~AssimpLoader();

	Model* importModel(const std::string& path, Shader* shader);
	Model* importAnimatedModel(const std::string& path, Shader* shader);
	std::vector<Model*> importScene(const std::string& path, Shader* shader);

	



private:

	
	Mesh* importMesh(const aiScene* scene);
	Animation* importAnimation(const aiScene* scene);


	const bool errorCheck(const aiScene* scene);

	Assimp::Importer m_importer;

};