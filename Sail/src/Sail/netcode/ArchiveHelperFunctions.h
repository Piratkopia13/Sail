#pragma once

#include "glm/vec3.hpp"
#include "Sail/../../libraries/cereal/types/vector.hpp"

namespace Archive {
	template<class Archive>
	void loadVec3(Archive& fromArchive, glm::vec3& vec3) {
		fromArchive(vec3.x, vec3.y, vec3.z);
	}

	template<class Archive>
	void archiveVec3(Archive& toArchive, const glm::vec3& vec3) {
		toArchive(vec3.x, vec3.y, vec3.z);
	}

	template<class Archive>
	void loadQuat(Archive& fromArchive, glm::quat& quat) {
		fromArchive(quat.x, quat.y, quat.z, quat.w);
	}

	template<class Archive>
	void archiveQuat(Archive& toArchive, const glm::quat& quat) {
		toArchive(quat.x, quat.y, quat.z, quat.w);
	}

}