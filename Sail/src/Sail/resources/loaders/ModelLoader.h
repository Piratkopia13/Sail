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

	std::shared_ptr<Mesh> getMesh();
	Entity::SPtr getEntity();

private:
	typedef void* SceneObject; // TODO: change
	void ParseNodesWithMeshes(const aiNode* node, SceneObject targetParent, const glm::mat4& accTransform);
	void ParseMeshes(const aiNode* node, SceneObject newObject);

private:
	std::string m_filepath;
	std::shared_ptr<Mesh> m_mesh;
	Entity::SPtr m_entity;
	const aiScene* m_scene;

};