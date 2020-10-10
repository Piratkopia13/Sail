#pragma once

#include <string>
#include "Sail/entities/Entity.h"

class Mesh;
struct aiScene;
struct aiNode;
struct aiMesh;

class ModelLoader {
public:
	ModelLoader(const std::string& filepath);
	~ModelLoader();

	Entity::SPtr getEntity();

private:
	Entity::SPtr parseNodesWithMeshes(const aiNode* node, Entity::SPtr parentEntity);
	Entity::SPtr parseMesh(const aiMesh* mesh, const glm::mat4& transform, const std::string& name);

private:
	std::string m_filepath;
	Entity::SPtr m_rootEntity;
	const aiScene* m_scene;

};