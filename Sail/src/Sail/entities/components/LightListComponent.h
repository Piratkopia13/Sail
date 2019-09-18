#pragma once
#include "Component.h"
#include "../../graphics/light/PointLight.h"
class LightListComponent : public Component<LightListComponent> {
public:
	LightListComponent() {}
	~LightListComponent() {}
	//contains a number of point lights in order to have multiple lights per entity
	std::vector<PointLight> m_pls;

private:

};