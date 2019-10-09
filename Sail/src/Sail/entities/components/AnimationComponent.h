#pragma once

#include "Component.h"
#include "Sail/graphics/geometry/Animation.h"
#include "Sail/api/Mesh.h"
#include "Sail/api/VertexBuffer.h"

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
		transformSize(0),
		computeUpdate(false),
		//transformedPositions(nullptr),
		//transformedNormals(nullptr),
		//transformedTangents(nullptr),
		//transformedBitangents(nullptr),
		//mesh(nullptr),
		m_stack(animationStack)
	{
		//mesh = std::unique_ptr<Mesh>(_mesh);
		transformSize = m_stack->getAnimation(0)->getAnimationTransformSize(unsigned int(0));
		transforms = SAIL_NEW glm::mat4[transformSize];
	}
	



	~AnimationComponent() {
		//Memory::SafeDeleteArr(transformedPositions);
		//Memory::SafeDeleteArr(transformedNormals);
		//Memory::SafeDeleteArr(transformedTangents);
		//Memory::SafeDeleteArr(transformedBitangents);

		Memory::SafeDeleteArr(data.indices);
		Memory::SafeDeleteArr(data.positions);
		Memory::SafeDeleteArr(data.normals);
		Memory::SafeDeleteArr(data.bitangents);
		Memory::SafeDeleteArr(data.colors);
		Memory::SafeDeleteArr(data.tangents);
		Memory::SafeDeleteArr(data.texCoords);
		Memory::SafeDeleteArr(transforms);

	}
	unsigned int dataSize;
	void resizeData(const unsigned int size) {
		dataSize = size;
		//transformedPositions = SAIL_NEW Mesh::vec3[size];
		//transformedNormals = SAIL_NEW Mesh::vec3[size];
		//transformedTangents = SAIL_NEW Mesh::vec3[size];
		//transformedBitangents = SAIL_NEW Mesh::vec3[size];


	}
	

	bool computeUpdate;
	float animationTime;
	unsigned int animationIndex;
	float animationSpeed;
	std::string animationName;
	Animation* currentAnimation;
	Animation* nextAnimation;
	bool blending;
	unsigned int transformSize;
	bool hasUpdated;

	class Transition {
	public:
		Transition(Animation* _to, const float time = 1.0f, const bool wait = true) {
			to = _to;
			transitionTime = time;
			transpiredTime = 0.0f;
			waitForEnd = wait;
		}
		Animation* to;
		float transitionTime;
		float transpiredTime;
		bool waitForEnd;
	};
	std::queue<Transition> transitions;

	Mesh::Data data;
	glm::mat4* transforms;

	std::unique_ptr<VertexBuffer> tposeVBuffer;

	AnimationStack* getAnimationStack() { return m_stack; };
private:
	AnimationStack* m_stack;
};