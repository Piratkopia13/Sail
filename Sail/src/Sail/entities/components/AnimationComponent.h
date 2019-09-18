#pragma once

#include "Component.h"
#include "Sail/graphics/geometry/Animation.h"

class AnimationComponent : public Component<AnimationComponent> {
public:
	//SAIL_COMPONENT 
	AnimationComponent(AnimationStack* animationStack) {}
	~AnimationComponent() {}


	float m_animationTime;
	unsigned int animationIndex;
	std::string animationName;

	


private:

	

	AnimationStack* stack;
	

};