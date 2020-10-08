#include "pch.h"
#include "ModelLoader.h"
#include "Sail/utils/Utils.h"

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
	m_model = std::make_unique<Model>(filepath);

	ParseNodesWithMeshes(m_scene->mRootNode, nullptr, glm::mat4(1.0f));

}

ModelLoader::~ModelLoader() {

}

std::shared_ptr<Model>& ModelLoader::getModel() {
	return m_model;
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
	for (uint32_t i = 0; i < node->mNumChildren; i++) {
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

	auto mesh = std::unique_ptr<Mesh>(Mesh::Create(buildData));
	m_model->addMesh(std::move(mesh));

}

