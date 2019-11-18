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

#ifdef DEVELOPMENT
	const unsigned int getByteSize() const override {
		return sizeof(*this) + sizeof(PointLight) * m_pls.size();
	}
#endif

private:
	//contains a number of point lights in order to have multiple lights per entity
	std::vector<PointLight> m_pls;

};