#pragma once

#include "Component.h"
#include "Sail/graphics/geometry/Animation.h"
#include "Sail/api/Mesh.h"

class AnimationComponent : public Component<AnimationComponent> {
public:
	//SAIL_COMPONENT 
	AnimationComponent(AnimationStack* animationStack) :
		animationTime(0),
		animationIndex(0),
		animationSpeed(1.0f),
		animationName(""),
		currentAnimation(nullptr),
		nextAnimation(nullptr),
		blending(false),
		dataSize(0),
		//transformedPositions(nullptr),
		//transformedNormals(nullptr),
		//transformedTangents(nullptr),
		//transformedBitangents(nullptr),
		//mesh(nullptr),
		m_stack(animationStack)
	{
		//mesh = std::unique_ptr<Mesh>(_mesh);
	}
	



	~AnimationComponent() {
		//Memory::SafeDeleteArr(transformedPositions);
		//Memory::SafeDeleteArr(transformedNormals);
		//Memory::SafeDeleteArr(transformedTangents);
		//Memory::SafeDeleteArr(transformedBitangents);
	}
	unsigned int dataSize;
	void resizeData(const unsigned int size) {
		dataSize = size;
		//transformedPositions = SAIL_NEW Mesh::vec3[size];
		//transformedNormals = SAIL_NEW Mesh::vec3[size];
		//transformedTangents = SAIL_NEW Mesh::vec3[size];
		//transformedBitangents = SAIL_NEW Mesh::vec3[size];


	}
	

	float animationTime;
	unsigned int animationIndex;
	float animationSpeed;
	std::string animationName;
	Animation* currentAnimation;
	Animation* nextAnimation;
	bool blending;

	//Mesh::vec3* transformedPositions;
	//Mesh::vec3* transformedNormals;
	//Mesh::vec3* transformedTangents;
	//Mesh::vec3* transformedBitangents;
	Mesh::Data data;



	AnimationStack* getAnimationStack() { return m_stack; };
	//std::unique_ptr<Mesh> mesh;
	

private:

	

	AnimationStack* m_stack;
	

};