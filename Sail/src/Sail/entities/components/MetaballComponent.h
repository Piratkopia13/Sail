#pragma once
#include "Component.h"
#include "Sail/graphics/geometry/PBRMaterial.h"

class MetaballComponent : public Component <MetaballComponent>{

public:
	MetaballComponent(float radius = 1.0f);
	~MetaballComponent();
	float getRadius() const;
	void setRadius(float radius);
	PBRMaterial* getMaterial();

private:
	float m_radius;
	PBRMaterial  m_material;
};

