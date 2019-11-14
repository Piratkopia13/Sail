#pragma once

#include "Component.h"
#include "Sail/graphics/geometry/Animation.h"
#include "Sail/api/Mesh.h"
#include "Sail/api/VertexBuffer.h"
#include <queue>
#include "Sail/entities/Entity.h"

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
		currentTransition(nullptr),
		blending(false),
		dataSize(0),
		transformSize(0),
		computeUpdate(true),
		animationW(0.0f), //TODO: REMOVE
		m_stack(animationStack),
		rightHandEntity(nullptr),
		leftHandEntity(nullptr),
		updateDT(true)
	{
		transformSize = m_stack->getAnimation(0)->getAnimationTransformSize(unsigned int(0));
		transforms = SAIL_NEW glm::mat4[transformSize];
	}
	



	~AnimationComponent() {

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
	float animationW;
	bool updateDT;
	
	// -0.55, 1.03, 0.11
	glm::mat4 rightHandPosition;
	// 0.57, 1.03, 0.11
	glm::mat4 leftHandPosition;

	glm::vec3 lPos;
	glm::vec3 rPos;
	
	Entity* rightHandEntity;
	Entity* leftHandEntity;
	void setAnimation(const unsigned int index) {
		animationIndex = index;
		if (animationIndex > m_stack->getAnimationCount()) {
			animationIndex = 0;
		}
		currentAnimation = m_stack->getAnimation(animationIndex);
	};
	void setAnimation(const std::string& name) {
		animationIndex = m_stack->getAnimationIndex(name);
		currentAnimation = m_stack->getAnimation(animationIndex);
	}

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
	Transition* currentTransition;

	Mesh::Data data;
	glm::mat4* transforms;

	std::unique_ptr<VertexBuffer> tposeVBuffer;

	AnimationStack* getAnimationStack() { return m_stack; };
#ifdef DEVELOPMENT
	void imguiRender(Entity** selected) {
		AnimationComponent* animationC = this;
		ImGui::Text("Animation: %s", animationC->currentAnimation->getName().c_str());
		ImGui::Checkbox("Update on GPU", &animationC->computeUpdate);
		if (ImGui::SliderFloat("Animation Speed", &animationC->animationSpeed, 0.0f, 3.0f)) {

		}
		AnimationStack* stack = animationC->getAnimationStack();
		float w = animationC->animationW;
		ImGui::SliderFloat("weight", &w, 0.0f, 1.0f);
		ImGui::Text("AnimationStack");
		for (unsigned int animationTrack = 0; animationTrack < stack->getAnimationCount(); animationTrack++) {
			float time = -1;
			if (animationC->currentAnimation == stack->getAnimation(animationTrack)) {
				time = animationC->animationTime;
			}
			if (animationC->nextAnimation == stack->getAnimation(animationTrack)) {
				if (time > -1) {
					float time2 = animationC->transitions.front().transpiredTime;
					ImGui::SliderFloat(std::string("CurrentTime: " + std::to_string(animationTrack) + "T").c_str(), &time2, 0.0f, stack->getAnimation(animationTrack)->getMaxAnimationTime());

				}
				else {
					time = animationC->transitions.front().transpiredTime;

				}
			}
			if (time == -1) {
				time = 0;

			}
			ImGui::SliderFloat(std::string("CurrentTime: " + std::to_string(animationTrack)).c_str(), &time, 0.0f, stack->getAnimation(animationTrack)->getMaxAnimationTime());
			if (animationC->currentAnimation == stack->getAnimation(animationTrack)) {
				animationC->animationTime = time;
			}
		}

		static float transitionTime = 0.4f;
		static bool transitionWait = false;
		ImGui::Checkbox("transition wait", &transitionWait);
		ImGui::SameLine();
		if (ImGui::SliderFloat("Transition Time", &transitionTime, 0.0f, 1.0f)) {

		}
		for (unsigned int animationIndex = 0; animationIndex < stack->getAnimationCount(); animationIndex++) {




			if (ImGui::Button(std::string("Switch to " + stack->getAnimation(animationIndex)->getName()).c_str())) {
				animationC->transitions.emplace(stack->getAnimation(animationIndex), transitionTime, transitionWait);
			}
			ImGui::Separator();
		}

	}
#endif


private:
	AnimationStack* m_stack;
};