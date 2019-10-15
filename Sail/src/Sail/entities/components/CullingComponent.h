#pragma once
#include "Component.h"

class CullingComponent : public Component<CullingComponent> {
public:
	CullingComponent() { }
	~CullingComponent() { }

	bool isVisible = true;
};