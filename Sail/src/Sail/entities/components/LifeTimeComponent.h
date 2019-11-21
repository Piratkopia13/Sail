#pragma once
#include "Component.h"

class LifeTimeComponent : public Component<LifeTimeComponent> {
public:
	LifeTimeComponent(float lifeTime = 1.0f) : totalLifeTime(lifeTime), elapsedTime(0.0f) {
	}
	~LifeTimeComponent() {}

	float totalLifeTime;
	float elapsedTime;
#ifdef DEVELOPMENT
	const unsigned int getByteSize() const override {
		return sizeof(*this);
	}
#endif
};