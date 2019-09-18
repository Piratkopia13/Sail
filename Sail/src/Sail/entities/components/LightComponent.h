#pragma once

#include "Component.h"
#include "../../graphics/light/PointLight.h"
class LightComponent : public Component<LightComponent> {
public:
	//create point light
	LightComponent(PointLight pl){
		m_pointLight.setAttenuation(pl.getAttenuation().constant,pl.getAttenuation().linear,pl.getAttenuation().quadratic);
		m_pointLight.setColor(pl.getColor());
		m_pointLight.setPosition(pl.getPosition());
		m_pointLight.setIndex(pl.getIndex());
	}
	~LightComponent() {}
	PointLight m_pointLight;
private:

};