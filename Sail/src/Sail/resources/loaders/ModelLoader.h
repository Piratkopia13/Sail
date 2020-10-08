#pragma once

#include <string>
#include "Sail/entities/Entity.h"

class Mesh;
struct aiScene;
struct aiNode;

class ModelLoader {
public:
	ModelLoader(const std::string& filepath);
	~ModelLoader();

	Entity::SPtr getEntity();

private:
	Entity::SPtr ParseNodesWithMeshes(const aiNode* node, Entity::SPtr parentEntity, const glm::mat4& accTransform);
	void ParseMeshes(const aiNode* node, Entity::SPtr newEntity);

private:
	std::string m_filepath;
	Entity::SPtr m_rootEntity;
	const aiScene* m_scene;

};