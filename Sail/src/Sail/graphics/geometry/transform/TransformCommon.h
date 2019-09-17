#pragma once

#include "pch.h"

struct TransformSnapshot {
	glm::vec3 m_translation;
	glm::vec3 m_rotation;
	glm::quat m_rotationQuat;
	glm::vec3 m_scale;
	glm::vec3 m_forward;
	glm::vec3 m_right;
	glm::vec3 m_up;

};

// TODO: add some wasUpdated bool so that 
struct TransformFrame {
	TransformSnapshot m_current;
	TransformSnapshot m_previous;

	//bool m_matNeedsUpdate;
	//bool m_parentUpdated;
	bool m_updatedDirections;
};