#include "pch.h"
#include "ModelLoader.h"
#include "Sail/utils/Utils.h"
#include "Sail/api/Mesh.h"
#include "Sail/entities/components/Components.h"
#include "../ResourceManager.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// For converting between ASSIMP and glm
static inline glm::vec4 vec4_cast(const aiColor3D& v) { return glm::vec4(v.r, v.g, v.b, 1.0f); }
static inline glm::vec4 vec4_cast(const aiVector3D& v) { return glm::vec4(v.x, v.y, v.z, 1.0f); }
static inline glm::vec3 vec3_cast(const aiVector3D& v) { return glm::vec3(v.x, v.y, v.z); }
static inline glm::vec2 vec2_cast(const aiVector3D& v) { return glm::vec2(v.x, v.y); }
static inline glm::quat quat_cast(const aiQuaternion& q) { return glm::quat(q.w, q.x, q.y, q.z); }
static inline glm::mat4 mat4_cast(const aiMatrix4x4& m) { return glm::transpose(glm::make_mat4(&m.a1)); }
static inline glm::mat4 mat4_cast(const aiMatrix3x3& m) { return glm::transpose(glm::make_mat3(&m.a1)); }

ModelLoader::ModelLoader(const std::string& filepath, Scene* scene, bool useAbsolutePath)
	: m_entityScene(scene)
{
	std::string path = filepath;
	if (!useAbsolutePath) {
		path = ResourceManager::DEFAULT_MODEL_LOCATION + path;
	}
	Assimp::Importer importer;
	import(&importer, path);

	m_rootEntity = parseNodesWithMeshes(m_scene->mRootNode, {});
	// Assimp names the root node "RootNode" - change it to the filepath instead
	m_rootEntity.setName(filepath);

}

ModelLoader::ModelLoader(const std::string& filepath) {
	Assimp::Importer importer;
	import(&importer, filepath);

	// Load a single mesh
	Mesh::Data buildData;
	getBuildData(m_scene->mMeshes[0], &buildData);
	m_mesh = std::shared_ptr<Mesh>(Mesh::Create(buildData));
}

ModelLoader::~ModelLoader() { }


Entity ModelLoader::getEntity() {
	return m_rootEntity;
}

Mesh::SPtr ModelLoader::getMesh() {
	return m_mesh;
}

void ModelLoader::import(Assimp::Importer* importer, const std::string& filepath) {
	// And have it read the given file with some example postprocessing
	// Usually - if speed is not the most important aspect for you - you'll
	// probably to request more postprocessing than we do in this example.
	m_scene = importer->ReadFile(filepath, 
		//aiProcess_FindInvalidData
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType |
		aiProcess_FlipUVs |
		aiProcess_ConvertToLeftHanded
	);
	// If the import failed, report it
	if (!m_scene) {
		Logger::Error(importer->GetErrorString());
		return;
	}
}

Entity ModelLoader::parseNodesWithMeshes(const aiNode* node, Entity::ID parentEntityId) {
	Entity parent;
	if (node->mNumMeshes == 0) {
		// Create an entity with a transform (but no mesh) to keep the hierarchy complete
		parent = parseMesh(nullptr, mat4_cast(node->mTransformation), std::string(node->mName.C_Str()));
		
		auto parentRelation = &parent.getComponent<RelationshipComponent>();
		parentRelation->parent = parentEntityId;
		if (parentEntityId) {
			auto& parentEntityRelation = Entity(parentEntityId, m_entityScene).getComponent<RelationshipComponent>();
			parentEntityRelation.numChildren++;
			if (!parentEntityRelation.first) parentEntityRelation.first = parent;
		}
	} else {
		Entity::ID lastParent;
		for (uint32_t i = 0; i < node->mNumMeshes; i++) {
			// All entities created here should have parentEntity as parent
			// their next/prev should also be set in this loop

			parent = parseMesh(m_scene->mMeshes[node->mMeshes[i]], mat4_cast(node->mTransformation), std::string(node->mName.C_Str()));

			auto parentRelation = &parent.getComponent<RelationshipComponent>();
			
			RelationshipComponent* lastRelation = nullptr;
			if (i > 0) lastRelation = &Entity(lastParent, m_entityScene).getComponent<RelationshipComponent>();

			parentRelation->parent = parentEntityId;
			if (parentEntityId) {
				auto& parentEntityRelation = Entity(parentEntityId, m_entityScene).getComponent<RelationshipComponent>();
				parentEntityRelation.numChildren++;
				if (i == 0 && !parentEntityRelation.first) parentEntityRelation.first = parent;
			}

			parentRelation->prev = lastParent;
			if (i > 0) lastRelation->next = parent;
			if (i > 0) assert(lastRelation->next < 50);
			
			lastParent = parent;
		}
	}

	Entity::ID lastChild;
	for (uint32_t i = 0; i < node->mNumChildren; i++) {
		// All entities created here should have parent as parent
		// their next/prev should also be set in this loop

		auto childEntity = parseNodesWithMeshes(node->mChildren[i], parent);

		auto* parentRelation = &childEntity.getComponent<RelationshipComponent>();

		RelationshipComponent* lastRelation = nullptr;
		if (i > 0) lastRelation = &Entity(lastChild, m_entityScene).getComponent<RelationshipComponent>();

		if (parentRelation->prev) {
			// A prev is already set, this means that the child had more than one mesh
			// We need to link the multiple meshes into the relation chain
			Entity firstInChildChain = Entity(parentRelation->prev, m_entityScene);
			RelationshipComponent* firstInChildChainRelation;
			while (true) {
				firstInChildChainRelation = &firstInChildChain.getComponent<RelationshipComponent>();
				if (firstInChildChainRelation->prev) firstInChildChain = Entity(firstInChildChainRelation->prev, m_entityScene);
				else break;
			}

			firstInChildChainRelation->prev = lastChild;
			if (i > 0) lastRelation->next = firstInChildChain;
		} else {
			parentRelation->prev = lastChild;
			if (i > 0) lastRelation->next = childEntity;
		}
		if (i > 0) assert(lastRelation->next < 50);

		lastChild = childEntity;
		lastRelation = parentRelation;
	}

	/*if (parentEntityRelation) {
		assert(parentEntityRelation->numChildren > 0 && parentEntityRelation->first && "Entity with children has no \"first\" relation set!");
	}*/

	return parent;
}

Entity ModelLoader::parseMesh(const aiMesh* mesh, const glm::mat4& transform, const std::string& name) {
	
	auto entity = m_entityScene->createEntity(name);
	entity.addComponent<TransformComponent>(transform);
	entity.addComponent<RelationshipComponent>();
	if (!mesh) return entity; // If no mesh, then just return with transform
	if (!((aiPrimitiveType)mesh->mPrimitiveTypes & aiPrimitiveType::aiPrimitiveType_TRIANGLE)) {
		Logger::Error("thats not a triangle");
	}

	// Create the mesh
	Mesh::Data buildData;
	getBuildData(mesh, &buildData);
	entity.addComponent<MeshComponent>(std::shared_ptr<Mesh>(Mesh::Create(buildData)));

	// Parse material

	auto pbrMat = entity.addComponent<MaterialComponent>().getAs<PBRMaterial>();

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

	static const bool IMPORT_TEXTURES = true;

#if 0
	Logger::Log("Mat properties:");
	for (uint32_t i = 0; i < meshMat->mNumProperties; i++) {
		auto prop = meshMat->mProperties[i];
		if (prop->mType == aiPTI_String)
			Logger::Log("\t[" + std::string(prop->mKey.C_Str()) + "] = " + std::string(((aiString*)prop->mData)->C_Str()));
		else if (prop->mType == aiPTI_Float)
			Logger::Log("\t[" + std::string(prop->mKey.C_Str()) + "] = " + std::to_string(*(float*)prop->mData));
		else if (prop->mType == aiPTI_Integer)
			Logger::Log("\t[" + std::string(prop->mKey.C_Str()) + "] = " + std::to_string(*(int*)prop->mData));
		else
			Logger::Log("\t[" + std::string(prop->mKey.C_Str()) + "]");
	}
#endif

	// Get Textures
	if (IMPORT_TEXTURES) {
		aiString texPath;
		if (meshMat->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), texPath) == AI_SUCCESS) {
			//Logger::Log("Diffuse: " + std::string(texPath.C_Str()));
			pbrMat->setAlbedoTexture(std::string(texPath.C_Str()));
			// Ignore underlying color if there is a diffuse/albedo texture set (blender seems to do this)
			pbrMat->setColor({ 1.f, 1.f, 1.f, color.a });
		}
		if (meshMat->Get(AI_MATKEY_TEXTURE_NORMALS(0), texPath) == AI_SUCCESS) {
			//Logger::Log("Normals: " + std::string(texPath.C_Str()));
			pbrMat->setNormalTexture(std::string(texPath.C_Str()));
		}
		if (meshMat->Get(AI_MATKEY_TEXTURE_SPECULAR(0), texPath) == AI_SUCCESS) {
			//Logger::Log("Specular: " + std::string(texPath.C_Str()));
		}
		if (meshMat->Get(AI_MATKEY_TEXTURE_OPACITY(0), texPath) == AI_SUCCESS) {
			pbrMat->enableTransparency(true);
		}
	}
	aiBlendMode blendMode;
	if (meshMat->Get(AI_MATKEY_BLEND_FUNC, blendMode) == AI_SUCCESS) {
		//Logger::Log(std::to_string(blendMode));
	}

	return entity;
}

void ModelLoader::getBuildData(const aiMesh* mesh, Mesh::Data* outBuildData) {
	outBuildData->numVertices = mesh->mNumVertices;
	outBuildData->numIndices = mesh->mNumFaces * 3; // assume 3 indices per face

	if (mesh->HasFaces())					outBuildData->indices = SAIL_NEW unsigned long[outBuildData->numIndices];
	if (mesh->HasPositions())				outBuildData->positions = SAIL_NEW Mesh::vec3[outBuildData->numVertices];
	if (mesh->HasNormals())					outBuildData->normals = SAIL_NEW Mesh::vec3[outBuildData->numVertices];
	if (mesh->HasTextureCoords(0))			outBuildData->texCoords = SAIL_NEW Mesh::vec2[outBuildData->numVertices];
	if (mesh->HasTangentsAndBitangents())	outBuildData->tangents = SAIL_NEW Mesh::vec3[outBuildData->numVertices];
	if (mesh->HasTangentsAndBitangents())	outBuildData->bitangents = SAIL_NEW Mesh::vec3[outBuildData->numVertices];

	for (uint32_t i = 0; i < outBuildData->numVertices; i++) {
		if (mesh->HasPositions())				outBuildData->positions[i].vec	= vec3_cast(mesh->mVertices[i]);
		if (mesh->HasNormals())					outBuildData->normals[i].vec	= vec3_cast(mesh->mNormals[i]);
		if (mesh->HasTextureCoords(0))			outBuildData->texCoords[i].vec	= vec3_cast(mesh->mTextureCoords[0][i]);
		if (mesh->HasTangentsAndBitangents())	outBuildData->tangents[i].vec	= vec3_cast(mesh->mTangents[i]);
		if (mesh->HasTangentsAndBitangents())	outBuildData->bitangents[i].vec = vec3_cast(mesh->mBitangents[i]);
	}

	if (mesh->HasFaces()) {
		uint32_t index = 0;
		for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
			auto& face = mesh->mFaces[i];
			for (uint32_t j = 0; j < face.mNumIndices; j++) {
				assert(face.mIndices[j] < outBuildData->numIndices && "This index is too large, that shouldn't have happened. Either model file is wrong or assimp is wrong.");
				outBuildData->indices[index++] = face.mIndices[j];
			}
		}
	}
}
