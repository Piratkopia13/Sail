#pragma once

#include "Sail.h"

class Object {
private:
	Transform m_transform;
protected:
	Model* model; 
	AABB* boundingBox; //Pointer to be able to detect if a bounding box has been made
	DirectX::SimpleMath::Vector4 lightColor;

public:
	Object();
	virtual ~Object();

	void setPosition(const DirectX::SimpleMath::Vector3 &newPosition);
	void updateBoundingBox(bool includeBoundingBoxRotation = true);
	void setModel(Model* model);
	void setLightColor(DirectX::SimpleMath::Vector4 color);
	Model* getModel();
	Transform& getTransform();
	AABB* getBoundingBox();
	DirectX::SimpleMath::Vector4 getLightColor();
	DirectX::SimpleMath::Vector4 getColor();

	virtual void draw() = 0;
};