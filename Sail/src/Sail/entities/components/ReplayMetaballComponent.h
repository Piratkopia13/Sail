#pragma once
#include "Component.h"
#include "Sail/graphics/geometry/PBRMaterial.h"

class ReplayMetaballComponent : public Component<ReplayMetaballComponent> {
public:
	ReplayMetaballComponent() : material(nullptr) {}
	~ReplayMetaballComponent() {}

	PBRMaterial material;
};

