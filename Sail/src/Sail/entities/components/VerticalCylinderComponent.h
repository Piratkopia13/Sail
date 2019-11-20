#pragma once
#include "Component.h"
#include "..//Physics/Cylinder.h"

class VerticalCylinderComponent : public Component<VerticalCylinderComponent> {
public:
	VerticalCylinderComponent() {}
	~VerticalCylinderComponent() {}

	VerticalCylinder cylinder;

#ifdef DEVELOPMENT
	const unsigned int getByteSize() const override {
		return sizeof(*this);
	}
#endif
};