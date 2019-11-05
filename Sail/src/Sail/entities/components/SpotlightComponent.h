#pragma once
#include "Component.h"
#include "../../graphics/light/PointLight.h"
//#include "../systems/light/SpotLightSystem.h"

class SpotLightSystem;

class SpotlightComponent final : public Component<SpotlightComponent> {
public:
	SpotlightComponent(){}
	~SpotlightComponent() {}

	SpotLight light; // Describes the light source when not rotated or moved
private:
	SpotLight light_entityRotated; //This one will be updated with entity transformations and submited to the renderer

	friend class SpotLightSystem;
	
};