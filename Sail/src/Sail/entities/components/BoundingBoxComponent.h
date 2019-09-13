#pragma once
#include "Component.h"
class Model;
class BoundingBox;

class BoundingBoxComponent : public Component<BoundingBoxComponent> {
public:
	BoundingBoxComponent(BoundingBox* boundingBox, Model* wireframe)
		: m_boundingBox(boundingBox)
		, m_wireframe(wireframe)
	{ }
	~BoundingBoxComponent() { delete m_boundingBox; }

	BoundingBox* getBoundingBox() const {
		return m_boundingBox;
	}

	Model* getWireframeModel() const {
		return m_wireframe;
	}

private:
	Model* m_wireframe;
	BoundingBox* m_boundingBox;
};