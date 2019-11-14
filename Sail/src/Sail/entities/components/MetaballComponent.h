#pragma once
#include "Component.h"
#include "Sail/graphics/geometry/PBRMaterial.h"

class MetaballComponent : public Component<MetaballComponent>{
public:
	MetaballComponent() : material(nullptr) {}
	~MetaballComponent() {}

	PBRMaterial material;
};

