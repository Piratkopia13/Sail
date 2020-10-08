#include "pch.h"
#include "ModelLoader.h"
#include "Sail/utils/Utils.h"
#include "Sail/api/Mesh.h"
#include "Sail/entities/components/Components.h"

#include <assimp/Importer.hpp> // C++ importer interface
#include <assimp/scene.h> // Output data structure
#include <assimp/postprocess.h> // Post processing flags

// For converting between ASSIMP and glm
static inline glm::vec3 vec3_cast(const aiVector3D& v) { return glm::vec3(v.x, v.y, v.z); }
static inline glm::vec2 vec2_cast(const aiVector3D& v) { return glm::vec2(v.x, v.y); }
static inline glm::quat quat_cast(const aiQuaternion& q) { return glm::quat(q.w, q.x, q.y, q.z); }
static inline glm::mat4 mat4_cast(const aiMatrix4x4& m) { return glm::transpose(glm::make_mat4(&m.a1)); }
static inline glm::mat4 mat4_cast(const aiMatrix3x3& m) { return glm::transpose(glm::make_mat3(&m.a1)); }

ModelLoader::ModelLoader(const std::string& filepath) {

	Assimp::Importer importer;
	// And have it read the given file with some example postprocessing
	// Usually - if speed is not the most important aspect for you - you'll
	// probably to request more postprocessing than we do in this example.
	m_scene = importer.ReadFile(filepath,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType |
		aiProcess_PreTransformVertices // TODO: remove this when a scenegraph exists
	);
	// If the import failed, report it
	if (!m_scene) {
		Logger::Error(importer.GetErrorString());
	}

	m_rootEntity = Entity::Create(filepath);
	ParseNodesWithMeshes(m_scene->mRootNode, m_rootEntity, glm::mat4(1.0f));

}

ModelLoader::~ModelLoader() {

}


Entity::SPtr ModelLoader::getEntity() {
	return m_rootEntity;
}

Entity::SPtr ModelLoader::ParseNodesWithMeshes(const aiNode* node, Entity::SPtr parentEntity, const glm::mat4& accTransform) {
	Entity::SPtr parent;
	glm::mat4 transform;
	// if node has meshes, create a new scene object for it
	if (node->mNumMeshes > 0) {
		auto newEntity = Entity::Create(node->mName.C_Str());
		newEntity->addComponent<TransformComponent>(mat4_cast(node->mTransformation));
		newEntity->addComponent<MaterialComponent<PBRMaterial>>();
		//targetParent.addChild(newObject);
		// copy the meshes
		ParseMeshes(node, newEntity);
		// the new object is the parent for all child nodes
		parent = newEntity;
		//transform.SetUnity();
	} else {
		// if no meshes, skip the node, but keep its transformation
		parent = parentEntity;
		transform = mat4_cast(node->mTransformation) * accTransform;
	}
	
	auto& parentRelation = parent->addComponent<RelationshipComponent>();
	if (parentEntity != parent)
		parentRelation->parent = parentEntity;
	Entity::SPtr newChildEntity = nullptr;
	RelationshipComponent::SPtr lastRelation = nullptr;

	parentRelation->numChildren = node->mNumChildren;

	// continue for all child nodes
	for (uint32_t i = 0; i < node->mNumChildren; i++) {
		auto& child = node->mChildren[i];
		auto previousChildEntity = newChildEntity;
		newChildEntity = ParseNodesWithMeshes(child, parent, transform);
		
		auto newRelation = newChildEntity->getComponent<RelationshipComponent>();
		newRelation->prev = previousChildEntity;
		
		if (i > 0) lastRelation->next = newChildEntity;

		if (i == 0) parentRelation->first = newChildEntity;

		lastRelation = newRelation;
	}
	return parent;
}

void ModelLoader::ParseMeshes(const aiNode* node, Entity::SPtr entity) {
	
	auto& aiMesh = m_scene->mMeshes[node->mMeshes[0]];

	Mesh::Data buildData;
	buildData.numVertices = aiMesh->mNumVertices;
	buildData.numIndices = aiMesh->mNumFaces * 3; // assume 3 indices per face

	buildData.indices = SAIL_NEW unsigned long[buildData.numIndices];
	buildData.positions = SAIL_NEW Mesh::vec3[buildData.numVertices];
	buildData.normals = SAIL_NEW Mesh::vec3[buildData.numVertices];
	buildData.texCoords = SAIL_NEW Mesh::vec2[buildData.numVertices];
	buildData.tangents = SAIL_NEW Mesh::vec3[buildData.numVertices];
	buildData.bitangents = SAIL_NEW Mesh::vec3[buildData.numVertices];

	for (uint32_t i = 0; i < buildData.numVertices; i++) {
		buildData.positions[i].vec = vec3_cast(aiMesh->mVertices[i]);
		buildData.normals[i].vec = vec3_cast(aiMesh->mNormals[i]);
		buildData.texCoords[i].vec = vec3_cast(aiMesh->mTextureCoords[0][i]);
		buildData.tangents[i].vec = vec3_cast(aiMesh->mTangents[i]);
		buildData.bitangents[i].vec = vec3_cast(aiMesh->mBitangents[i]);
	}

	uint32_t index = 0;
	for (uint32_t i = 0; i < aiMesh->mNumFaces; i++) {
		auto& face = aiMesh->mFaces[i];
		for (uint32_t j = 0; j < face.mNumIndices; j++) {
			buildData.indices[index++] = face.mIndices[j];
		}
	}

	entity->addComponent<MeshComponent>(std::shared_ptr<Mesh>(Mesh::Create(buildData)));

}