#pragma once

#include <glm/glm.hpp>
#include "Sail.h"

class Object {
private:
	Transform m_transform;
protected:
	Model* model; 
	AABB* boundingBox; //Pointer to be able to detect if a bounding box has been made
	glm::vec4 lightColor;

public:
	Object();
	virtual ~Object();

	void setPosition(const glm::vec3 &newPosition);
	void updateBoundingBox(bool includeBoundingBoxRotation = true);
	void setModel(Model* model);
	void setLightColor(glm::vec4 color);
	Model* getModel();
	Transform& getTransform();
	AABB* getBoundingBox();
	glm::vec4 getLightColor();
	glm::vec4 getColor();

	virtual void draw() = 0;
};