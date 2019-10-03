#pragma once
#include "Component.h"
#include "Sail/graphics/geometry/PhongMaterial.h"

class MetaballComponent : public Component <MetaballComponent>{

public:
	MetaballComponent(float radius = 1.0f);
	~MetaballComponent();
	float getRadius() const;
	void setRadius(float radius);
	PhongMaterial* getMaterial();

private:
	float m_radius;
	PhongMaterial m_material;
};

