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

				Logger::Log("vel: " + std::to_string(moveC->velocity.length()));
			if (moveC->velocity.length() > 0.5f ) {
				animationC->setAnimation(1);
				//animationC->animationTime = 0.0f;
			}
			else {
				animationC->setAnimation(2);
				//animationC->animationTime = 0.0f;



			}





		}
		else {
			int i = 0;
		}


	}



}
