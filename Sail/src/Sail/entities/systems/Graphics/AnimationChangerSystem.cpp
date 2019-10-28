#include "pch.h"
#include "AnimationChangerSystem.h"
#include "../../ECS.h"
#include "../../components/TransformComponent.h"
#include "../../components/AnimationComponent.h"
#include "../../components/MovementComponent.h"

AnimationChangerSystem::AnimationChangerSystem() {
	registerComponent<AnimationComponent>(true, true, true);
	registerComponent<MovementComponent>(true, true, false);
}



AnimationChangerSystem::~AnimationChangerSystem() {

}

void AnimationChangerSystem::update(float dt) {

	for (auto& e : entities) {
		AnimationComponent* animationC = e->getComponent<AnimationComponent>();
		MovementComponent* moveC = e->getComponent<MovementComponent>();
		if (animationC && moveC) {

			if (glm::length(moveC->velocity) < 0.1f) {
				animationC->setAnimation(IDLE);
			}
			else {
				if (moveC->relVel.z > 0.1f) {
					animationC->setAnimation(RIGHT);
				}
				else if (moveC->relVel.z < -0.1f) {
					animationC->setAnimation(LEFT);
				}
				if (moveC->relVel.x > 0.1f) {
					animationC->setAnimation(FORWARD);
				} 
				else if (moveC->relVel.x < -0.1f) {
					animationC->setAnimation(BACKWARD);
				}
			}
		}
		else {
			int i = 0;
		}
	}
}
