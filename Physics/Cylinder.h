#pragma once
#include <glm/vec3.hpp>

class Cylinder {
public:
	// Defaults to a vertical cylinder
	Cylinder() : position(glm::vec3(0)), radius(1), halfHeight(1), centralAxis(glm::vec3(0, 1, 0)) {
	}
	~Cylinder() {
	}

	glm::vec3 position;
	float radius;
	float halfHeight;
	glm::vec3 centralAxis;
};


class VerticalCylinder {
public:
	VerticalCylinder() : position(glm::vec3(0)), radius(1), halfHeight(1) {
	}
	~VerticalCylinder() {
	}

	glm::vec3 position;
	float radius;
	float halfHeight;
};