#include "pch.h"
#include "AnimationInitSystem.h"
#include "Sail/graphics/shader/dxr/GBufferOutShader.h"
#include "../../ECS.h"
#include "Sail/Application.h"
#include "../../components/TransformComponent.h"
#include "../../components/ModelComponent.h"
#include "../../components/AnimationComponent.h"
#include "../../components/NoEntityComponent.h"

AnimationInitSystem::AnimationInitSystem() {
	registerComponent<NoEntityComponent>(true, false, false);
}

AnimationInitSystem::~AnimationInitSystem() {
}

void AnimationInitSystem::loadAnimations() {

}

void AnimationInitSystem::initAnimations() {
	Application* app = Application::getInstance();
	auto* shader = &app->getResourceManager().getShaderSet<GBufferOutShader>();

	auto animationEntity2 = ECS::Instance()->createEntity("DocGunRun4");
	animationEntity2->addComponent<TransformComponent>();
	animationEntity2->getComponent<TransformComponent>()->translate(0, 0, 0);
	animationEntity2->addComponent<ModelComponent>(&app->getResourceManager().getModelCopy("DocGunRun4.fbx"));
	animationEntity2->getComponent<ModelComponent>()->getModel()->setIsAnimated(true);
	animationEntity2->addComponent<AnimationComponent>(&app->getResourceManager().getAnimationStack("DocGunRun4.fbx"));
	animationEntity2->getComponent<AnimationComponent>()->currentAnimation = animationEntity2->getComponent<AnimationComponent>()->getAnimationStack()->getAnimation(0);

}
