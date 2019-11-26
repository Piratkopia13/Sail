#pragma once
#include "Component.h"
#include "Sail/graphics/geometry/PBRMaterial.h"

class MetaballComponent : public Component<MetaballComponent>{
public:
	MetaballComponent() : material(nullptr) {}
	~MetaballComponent() {}

	PBRMaterial material;
	int renderGroupIndex = 0;

#ifdef DEVELOPMENT
	const unsigned int getByteSize() const override {
		return sizeof(*this);
	}
#endif
};

