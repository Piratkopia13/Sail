#pragma once
#include "Component.h"
#include "../Physics/BoundingBox.h"
class Model;

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

private:
	Model* m_wireframe;
	BoundingBox m_boundingBox;
};