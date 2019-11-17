#pragma once
#include "Component.h"
#include "../Physics/BoundingBox.h"

#include "Sail/graphics/geometry/Transform.h"
class Model;

class BoundingBoxComponent : public Component<BoundingBoxComponent> {
public:
	BoundingBoxComponent() {

	}
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

//#ifdef DEVELOPMENT
//	void imguiRender() {
//		
//	}
//#endif

private:
	Model* m_wireframe = nullptr;
	BoundingBox m_boundingBox;
	Transform m_transform;
};