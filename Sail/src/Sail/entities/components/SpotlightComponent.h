#pragma once
#include "Component.h"
#include "../../graphics/light/PointLight.h"

class SpotLightSystem;

class SpotlightComponent final : public Component<SpotlightComponent> {
public:
	SpotlightComponent(){}
	~SpotlightComponent() {}

	bool isOn;
	int roomID;
	float activeTimer;
	float activeLimit = 5.f;
	SpotLight light; // Describes the light source when not rotated or moved
private:
	SpotLight m_lightEntityRotated; //This one will be updated with entity transformations and submited to the renderer

	friend class SpotLightSystem;
	
};
