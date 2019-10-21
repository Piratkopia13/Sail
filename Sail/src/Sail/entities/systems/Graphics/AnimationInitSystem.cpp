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
	std::string name = "Docfbx.fbx";

	AnimationStack* stack = &app->getResourceManager().getAnimationStack(name);
	unsigned int animationCount = stack->getAnimationCount();
	for (unsigned int i = 0; i < animationCount; i++) {
		auto animationEntity2 = ECS::Instance()->createEntity("Doc" + std::to_string(i));
		animationEntity2->addComponent<TransformComponent>();
		animationEntity2->getComponent<TransformComponent>()->translate(-2.0f+i*1.2f, 0, 0);
		Model* model = &app->getResourceManager().getModelCopy(name);
		animationEntity2->addComponent<ModelComponent>(model);
		model->getMesh(0)->getMaterial()->setMetalnessRoughnessAOTexture("pbr/Character/CharacterMRAO.tga");
		model->getMesh(0)->getMaterial()->setAlbedoTexture("pbr/Character/CharacterTex.tga");
		model->getMesh(0)->getMaterial()->setNormalTexture("pbr/Character/CharacterNM.tga");
		animationEntity2->getComponent<ModelComponent>()->getModel()->setIsAnimated(true);
		animationEntity2->addComponent<AnimationComponent>(stack);
		animationEntity2->getComponent<AnimationComponent>()->currentAnimation = stack->getAnimation(i);

	}

}
