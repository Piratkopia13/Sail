#pragma once

#include "Component.h"
#include "Sail/graphics/geometry/Animation.h"
#include "Sail/api/Mesh.h"

class AnimationComponent : public Component<AnimationComponent> {
public:
	//SAIL_COMPONENT 
	AnimationComponent(AnimationStack* animationStack, Mesh* _mesh) :
		animationTime(0),
		animationIndex(0),
		animationSpeed(1.0f),
		animationName(""),
		currentAnimation(nullptr),
		nextAnimation(nullptr),
		blending(false),
		mesh(nullptr),
		m_stack(animationStack)
	
	{
		mesh = std::unique_ptr<Mesh>(_mesh);
	}
	~AnimationComponent() {}


	float animationTime;
	unsigned int animationIndex;
	float animationSpeed;
	std::string animationName;
	Animation* currentAnimation;
	Animation* nextAnimation;
	bool blending;

	AnimationStack* getAnimationStack() { return m_stack; };
	std::unique_ptr<Mesh> mesh;
	

private:

	

	AnimationStack* m_stack;
	

};