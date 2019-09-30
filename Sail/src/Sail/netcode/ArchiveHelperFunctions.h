#pragma once

#include "glm/vec3.hpp"
#include "Sail/../../libraries/cereal/types/vector.hpp"

namespace Archive {
	template<class Archive>
	void serializeVec3(Archive& ar, glm::vec3& vec3) {
		ar(vec3.x, vec3.y, vec3.z);
	}

	template<class Archive>
	void serealizeQuat(Archive& ar, glm::quat& quat) {
		ar(quat.x, quat.y, quat.z, quat.w);
	}

}