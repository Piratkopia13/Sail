#pragma once
#include "Component.h"
#include "../Physics/BoundingBox.h"
class Model;
class Transform;

class BoundingBoxComponent : public Component<BoundingBoxComponent> {
public:
	BoundingBoxComponent(Model* wireframe)
		: m_wireframe(wireframe)
	{ }
	~BoundingBoxComponent() { }

	BoundingBox* getBoundingBox() {
		return &m_boundingBox;
	}

	Model* getWireframeModel() const {
		return m_wireframe;
	}

	Transform* getTransform() {
		return &m_transform;
	}

private:
	Model* m_wireframe;
	BoundingBox m_boundingBox;
	Transform m_transform;
};