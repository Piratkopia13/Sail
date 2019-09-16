#pragma once
#include "BaseComponentSystem.h"

template<typename T>
class ComponentSystem : public BaseComponentSystem {
public:
	virtual ~ComponentSystem() {}

protected:
	ComponentSystem() {}
};