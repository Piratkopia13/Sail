#pragma once

#include <string>
#include "Sail/entities/Entity.h"
#include "Sail/api/Mesh.h"

struct aiScene;
struct aiNode;
struct aiMesh;
namespace Assimp {
	class Importer;
}

class ModelLoader {
public:
	// Creates an entity in the given scene with child-entities for each mesh in the model
	ModelLoader(const std::string& filepath, Scene* scene);
	// Loads only the first mesh in the model
	ModelLoader(const std::string& filepath);

	~ModelLoader();

	Entity getEntity();
	Mesh::SPtr getMesh();

private:
	void import(Assimp::Importer* importer, const std::string& filepath);
	Entity parseNodesWithMeshes(const aiNode* node, Entity::ID parentEntityId);
	Entity parseMesh(const aiMesh* mesh, const glm::mat4& transform, const std::string& name);

	void getBuildData(const aiMesh* mesh, Mesh::Data* outBuildData);

private:
	std::string m_filepath;
	Entity m_rootEntity;
	const aiScene* m_scene;
	Scene* m_entityScene;
	
	Mesh::SPtr m_mesh; // Only used for the constructor importing a single mesh

};