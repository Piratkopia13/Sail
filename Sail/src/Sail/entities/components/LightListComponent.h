#pragma once
#include "Component.h"
#include "../../graphics/light/PointLight.h"

class LightListComponent : public Component<LightListComponent> {
public:
	LightListComponent() {}
	~LightListComponent() {}

	std::vector<PointLight>& getLightList() {
		return m_pls;
	}

private:
	//contains a number of point lights in order to have multiple lights per entity
	std::vector<PointLight> m_pls;

};