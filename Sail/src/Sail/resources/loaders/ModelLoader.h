#pragma once

#include <string>
#include "../../graphics/geometry/Model.h"

struct aiScene;
struct aiNode;

class ModelLoader {
public:
	ModelLoader(const std::string& filepath);
	~ModelLoader();

	std::shared_ptr<Model>& getModel();

private:
	typedef void* SceneObject; // TODO: change
	void ParseNodesWithMeshes(const aiNode* node, SceneObject targetParent, const glm::mat4& accTransform);
	void ParseMeshes(const aiNode* node, SceneObject newObject);

private:
	std::string m_filepath;
	std::shared_ptr<Model> m_model;
	const aiScene* m_scene;

};