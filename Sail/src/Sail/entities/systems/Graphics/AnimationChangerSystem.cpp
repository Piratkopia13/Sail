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
			
			if (animationC->animationIndex == IDLEJUMP && animationC->animationTime < 0.1f) {
				animationC->setAnimation(IDLE);
			}

			if (glm::length(moveC->velocity) < 0.1f && animationC->animationIndex != IDLEJUMP) {
				animationC->setAnimation(IDLE);
			} 
			else if (moveC->relVel.y > 0.1f && fabsf(moveC->relVel.x) < 0.1f && fabsf(moveC->relVel.z) < 0.1f) {
				if (animationC->animationIndex != IDLEJUMP) {
					animationC->setAnimation(IDLEJUMP);
					animationC->animationTime = 0.310f;
				}
				
			}
			else {

				if (animationC->animationIndex == FORWARDJUMP) {
					if (animationC->animationTime < 0.1f) {
						animationC->setAnimation(IDLE);
					}
					else {
						continue;
					}
				}

				
				if (moveC->relVel.y > 0.1f && (fabsf(moveC->relVel.x) > 0.1f || fabsf(moveC->relVel.z) > 0.1f)) {
					animationC->setAnimation(FORWARDJUMP);
					animationC->animationTime = 0.5f;
				}
				else if (fabsf(moveC->relVel.x) > fabsf(moveC->relVel.z)) {
					if (moveC->relVel.x > 0.1f) {
						animationC->setAnimation(FORWARD);
					} 
					else if (moveC->relVel.x < -0.1f) {
						animationC->setAnimation(BACKWARD);
					}
				} 
				else {
					if (moveC->relVel.z > 0.1f) {
						animationC->setAnimation(RIGHT);
					} else if (moveC->relVel.z < -0.1f) {
						animationC->setAnimation(LEFT);
					}
				}
			}
		}
		else {
			int i = 0;
		}
	}
}
