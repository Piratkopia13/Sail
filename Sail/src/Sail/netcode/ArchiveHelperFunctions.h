#pragma once

#include "glm/vec3.hpp"
#include "ArchiveTypes.h"
#include "Sail/../../libraries/cereal/types/vector.hpp"


/*
  Helper functions to serialize various data structs/classes that aren't in STL
*/
namespace ArchiveHelpers {

	// glm::vec3
	template<class Archive>
	void loadVec3(Archive& fromArchive, glm::vec3& vec3)     { fromArchive(vec3.x, vec3.y, vec3.z); }

	template<class Archive>
	void saveVec3(Archive& toArchive, const glm::vec3& vec3) { toArchive(vec3.x, vec3.y, vec3.z); }


	// glm::quat
	template<class Archive>
	void loadQuat(Archive& fromArchive, glm::quat& quat)     { fromArchive(quat.x, quat.y, quat.z, quat.w); }

	template<class Archive>
	void saveQuat(Archive& toArchive, const glm::quat& quat) { toArchive(quat.x, quat.y, quat.z, quat.w); }


	// glm::mat4
	template<class Archive>
	void loadMat4(Archive& fromArchive, glm::mat4& mat) {
		fromArchive(
			mat[0][0], mat[1][0], mat[2][0], mat[3][0],
			mat[0][1], mat[1][1], mat[2][1], mat[3][1],
			mat[0][2], mat[1][2], mat[2][2], mat[3][2],
			mat[0][3], mat[1][3], mat[2][3], mat[3][3]
		);
	}

	template<class Archive>
	void saveMat4(Archive& toArchive, const glm::mat4& mat) {
		toArchive(
			mat[0][0], mat[1][0], mat[2][0], mat[3][0],
			mat[0][1], mat[1][1], mat[2][1], mat[3][1],
			mat[0][2], mat[1][2], mat[2][2], mat[3][2],
			mat[0][3], mat[1][3], mat[2][3], mat[3][3]
		);
	}
}