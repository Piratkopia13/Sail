#pragma once

#include <string>
#include "../../graphics/geometry/Model.h"

#include <assimp/Importer.hpp> // C++ importer interface
#include <assimp/scene.h> // Output data structure
#include <assimp/postprocess.h> // Post processing flags

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
	// For converting between ASSIMP and glm
	static inline glm::vec3 vec3_cast(const aiVector3D& v) { return glm::vec3(v.x, v.y, v.z); }
	static inline glm::vec2 vec2_cast(const aiVector3D& v) { return glm::vec2(v.x, v.y); }
	static inline glm::quat quat_cast(const aiQuaternion& q) { return glm::quat(q.w, q.x, q.y, q.z); }
	static inline glm::mat4 mat4_cast(const aiMatrix4x4& m) { return glm::transpose(glm::make_mat4(&m.a1)); }
	static inline glm::mat4 mat4_cast(const aiMatrix3x3& m) { return glm::transpose(glm::make_mat3(&m.a1)); }

private:
	std::string m_filepath;
	std::shared_ptr<Model> m_model;
	const aiScene* m_scene;

};