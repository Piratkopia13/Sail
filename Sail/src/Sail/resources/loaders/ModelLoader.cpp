#include "pch.h"
#include "ModelLoader.h"
#include "Sail/utils/Utils.h"
#include "Sail/api/Mesh.h"
#include "Sail/entities/components/Components.h"

#include <assimp/Importer.hpp> // C++ importer interface
#include <assimp/scene.h> // Output data structure
#include <assimp/postprocess.h> // Post processing flags

// For converting between ASSIMP and glm
static inline glm::vec4 vec4_cast(const aiColor3D& v) { return glm::vec4(v.r, v.g, v.b, 1.0f); }
static inline glm::vec4 vec4_cast(const aiVector3D& v) { return glm::vec4(v.x, v.y, v.z, 1.0f); }
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
		aiProcess_FlipUVs |
		aiProcess_ConvertToLeftHanded
	);
	// If the import failed, report it
	if (!m_scene) {
		Logger::Error(importer.GetErrorString());
		return;
	}

	m_rootEntity = parseNodesWithMeshes(m_scene->mRootNode, {});
	m_rootEntity->setName(filepath);

}

ModelLoader::~ModelLoader() { }


Entity::SPtr ModelLoader::getEntity() {
	return m_rootEntity;
}

Entity::SPtr ModelLoader::parseNodesWithMeshes(const aiNode* node, Entity::SPtr parentEntity) { 
	auto parentEntityRelation = (parentEntity) ? parentEntity->getComponent<RelationshipComponent>() : nullptr;

	Entity::SPtr parent;
	if (node->mNumMeshes == 0) {
		// Create an entity with a transform (but no mesh) to keep the hierarchy complete
		parent = parseMesh(nullptr, mat4_cast(node->mTransformation), std::string(node->mName.C_Str()));
		
		auto parentRelation = parent->getComponent<RelationshipComponent>();
		parentRelation->parent = parentEntity;
		if (parentEntityRelation) {
			parentEntityRelation->numChildren++;
			if (!parentEntityRelation->first) parentEntityRelation->first = parent;
		}
	} else {
		Entity::SPtr lastParent;
		RelationshipComponent::SPtr lastRelation;
		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			// All entities created here should have parentEntity as parent
			// their next/prev should also be set in this loop

			parent = parseMesh(m_scene->mMeshes[node->mMeshes[i]], mat4_cast(node->mTransformation), std::string(node->mName.C_Str()));

			auto parentRelation = parent->getComponent<RelationshipComponent>();
			parentRelation->parent = parentEntity;
			if (parentEntityRelation) {
				parentEntityRelation->numChildren++;
				if (i == 0 && !parentEntityRelation->first) parentEntityRelation->first = parent;
			}

			parentRelation->prev = lastParent;
			if (i > 0) lastRelation->next = parent;
			
			lastParent = parent;
			lastRelation = parentRelation;
		}
	}

	Entity::SPtr lastChild;
	RelationshipComponent::SPtr lastRelation;
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		// All entities created here should have parent as parent
		// their next/prev should also be set in this loop

		auto childEntity = parseNodesWithMeshes(node->mChildren[i], parent);

		auto parentRelation = childEntity->getComponent<RelationshipComponent>();

		if (parentRelation->prev) {
			// A prev is already set, this means that the child had more than one mesh
			// We need to link the multiple meshes into the relation chain
			Entity::SPtr firstInChildChain = parentRelation->prev;
			RelationshipComponent::SPtr firstInChildChainRelation;
			while (true) {
				firstInChildChainRelation = firstInChildChain->getComponent<RelationshipComponent>();
				if (firstInChildChainRelation->prev) firstInChildChain = firstInChildChainRelation->prev;
				else break;
			}

			firstInChildChainRelation->prev = lastChild;
			if (i > 0) lastRelation->next = firstInChildChain;
		} else {
			parentRelation->prev = lastChild;
			if (i > 0) lastRelation->next = childEntity;
		}

		lastChild = childEntity;
		lastRelation = parentRelation;
	}

	/*if (parentEntityRelation) {
		assert(parentEntityRelation->numChildren > 0 && parentEntityRelation->first && "Entity with children has no \"first\" relation set!");
	}*/

	return parent;
}

Entity::SPtr ModelLoader::parseMesh(const aiMesh* mesh, const glm::mat4& transform, const std::string& name) {
	
	auto entity = Entity::Create(name);
	entity->addComponent<TransformComponent>(transform);
	entity->addComponent<RelationshipComponent>();
	if (!mesh) return entity; // If no mesh, then just return with transform

	// Create the mesh
	Mesh::Data buildData;
	buildData.numVertices = mesh->mNumVertices;
	buildData.numIndices = mesh->mNumFaces * 3; // assume 3 indices per face

	buildData.indices = SAIL_NEW unsigned long[buildData.numIndices];
	buildData.positions = SAIL_NEW Mesh::vec3[buildData.numVertices];
	buildData.normals = SAIL_NEW Mesh::vec3[buildData.numVertices];
	buildData.texCoords = SAIL_NEW Mesh::vec2[buildData.numVertices];
	buildData.tangents = SAIL_NEW Mesh::vec3[buildData.numVertices];
	buildData.bitangents = SAIL_NEW Mesh::vec3[buildData.numVertices];

	for (uint32_t i = 0; i < buildData.numVertices; i++) {
		buildData.positions[i].vec = vec3_cast(mesh->mVertices[i]);
		buildData.normals[i].vec = vec3_cast(mesh->mNormals[i]);
		buildData.texCoords[i].vec = vec3_cast(mesh->mTextureCoords[0][i]);
		buildData.tangents[i].vec = vec3_cast(mesh->mTangents[i]);
		buildData.bitangents[i].vec = vec3_cast(mesh->mBitangents[i]);
	}

	uint32_t index = 0;
	for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
		auto& face = mesh->mFaces[i];
		for (uint32_t j = 0; j < face.mNumIndices; j++) {
			buildData.indices[index++] = face.mIndices[j];
		}
	}

	entity->addComponent<MeshComponent>(std::shared_ptr<Mesh>(Mesh::Create(buildData)));

	// Parse material
	auto pbrMat = entity->addComponent<MaterialComponent<PBRMaterial>>()->get();

	auto matIndex = mesh->mMaterialIndex;
	auto meshMat = m_scene->mMaterials[matIndex];
	
	glm::vec4 color(1.0f);
	// Get diffuse color
	aiColor3D diffuseColor;
	if (meshMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == AI_SUCCESS) {
		color = vec4_cast(diffuseColor);
	}
	// Get opacity
	float opacity;
	if (meshMat->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS) {
		color.a = opacity;
	}
	pbrMat->setColor(color);

	// Get shininess
	float shininess;
	if (meshMat->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
		//pbrMat->setRoughnessScale(1.f - shininess);
	}

	Logger::Log("NewMat");

	// Get Textures
	aiString texPath;
	if (meshMat->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), texPath) == AI_SUCCESS) {
		Logger::Log("Diffuse: " + std::string(texPath.C_Str()));
		pbrMat->setAlbedoTexture(std::string(texPath.C_Str()));
	}
	if (meshMat->Get(AI_MATKEY_TEXTURE_NORMALS(0), texPath) == AI_SUCCESS) {
		Logger::Log("Normals: " + std::string(texPath.C_Str()));
		pbrMat->setNormalTexture(std::string(texPath.C_Str()));
	}
	if (meshMat->Get(AI_MATKEY_TEXTURE_SPECULAR(0), texPath) == AI_SUCCESS) {
		Logger::Log("Specular: " + std::string(texPath.C_Str()));
	}
	if (meshMat->Get(AI_MATKEY_TEXTURE_OPACITY(0), texPath) == AI_SUCCESS) {
		pbrMat->enableTransparency(true);
	}

	return entity;
}