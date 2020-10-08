#include "pch.h"
#include "ModelLoader.h"
#include "Sail/utils/Utils.h"
#include "Sail/api/Mesh.h"

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

	ParseNodesWithMeshes(m_scene->mRootNode, nullptr, glm::mat4(1.0f));

}

ModelLoader::~ModelLoader() {

}

std::shared_ptr<Mesh> ModelLoader::getMesh() {
	return m_mesh;
}

Entity::SPtr ModelLoader::getEntity() {
	return m_entity;
}

void ModelLoader::ParseNodesWithMeshes(const aiNode* node, SceneObject targetParent, const glm::mat4& accTransform) {
	SceneObject parent;
	glm::mat4 transform;
	// if node has meshes, create a new scene object for it
	if (node->mNumMeshes > 0) {
		//SceneObject newObject = new SceneObject;
		SceneObject newObject = nullptr;
		//targetParent.addChild(newObject);
		// copy the meshes
		ParseMeshes(node, newObject);
		// the new object is the parent for all child nodes
		parent = newObject;
		//transform.SetUnity();
	} else {
		// if no meshes, skip the node, but keep its transformation
		parent = targetParent;
		transform = mat4_cast(node->mTransformation) * accTransform;
	}
	// continue for all child nodes
	for (uint32_t i = 0; i < glm::min<int>(node->mNumChildren, 1); i++) {
		//for (uint32_t i = 0; i < node->mNumChildren; i++) {
		auto& child = node->mChildren[i];
		ParseNodesWithMeshes(child, parent, transform);
	}
}

void ModelLoader::ParseMeshes(const aiNode* node, SceneObject newObject) {
	
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

	m_mesh = std::shared_ptr<Mesh>(Mesh::Create(buildData));

}