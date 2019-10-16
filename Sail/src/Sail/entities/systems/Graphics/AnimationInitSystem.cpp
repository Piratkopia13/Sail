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
	Application* app = Application::getInstance();
	auto* shader = &app->getResourceManager().getShaderSet<GBufferOutShader>();
	app->getResourceManager().loadModel("AnimationTest/walkTri.fbx", shader, ResourceManager::ImporterType::SAIL_FBXSDK);
	//animatedModel->getMesh(0)->getMaterial()->setDiffuseTexture("sponza/textures/character1texture.tga");

#ifndef _DEBUG
	//m_app->getResourceManager().loadModel("AnimationTest/ScuffedSteve_2.fbx", shader, ResourceManager::ImporterType::SAIL_FBXSDK);
	//m_app->getResourceManager().loadModel("AnimationTest/BaseMesh_Anim.fbx", shader, ResourceManager::ImporterType::SAIL_FBXSDK);
	app->getResourceManager().loadModel("AnimationTest/DEBUG_BALLBOT.fbx", shader, ResourceManager::ImporterType::SAIL_FBXSDK);
#endif
}

void AnimationInitSystem::initAnimations() {
	Application* app = Application::getInstance();
	auto* shader = &app->getResourceManager().getShaderSet<GBufferOutShader>();

	auto animationEntity2 = ECS::Instance()->createEntity("animatedModel2");
	animationEntity2->addComponent<TransformComponent>();
	animationEntity2->getComponent<TransformComponent>()->translate(-5, 0, 0);
	animationEntity2->getComponent<TransformComponent>()->translate(100.f, 100.f, 100.f);
	animationEntity2->addComponent<ModelComponent>(&app->getResourceManager().getModelCopy("AnimationTest/walkTri.fbx"));
	animationEntity2->getComponent<ModelComponent>()->getModel()->setIsAnimated(true);
	animationEntity2->addComponent<AnimationComponent>(&app->getResourceManager().getAnimationStack("AnimationTest/walkTri.fbx"));
	animationEntity2->getComponent<AnimationComponent>()->currentAnimation = animationEntity2->getComponent<AnimationComponent>()->getAnimationStack()->getAnimation(0);

	std::string animName = "";
#ifndef _DEBUG
	animName = "AnimationTest/DEBUG_BALLBOT.fbx";
	unsigned int count = app->getResourceManager().getAnimationStack(animName).getAnimationCount();
	for (int i = 0; i < 2; i++) {
		auto animationEntity5 = ECS::Instance()->createEntity("DEBUG_BALLBOT-" + std::to_string(i));
		animationEntity5->addComponent<TransformComponent>();
		animationEntity5->getComponent<TransformComponent>()->translate(1.0f + (i * 2), 1, 0);
		//animationEntity5->getComponent<TransformComponent>()->rotateAroundX(-3.14f*0.5f);
		animationEntity5->getComponent<TransformComponent>()->scale(0.005f);
		animationEntity5->addComponent<ModelComponent>(&app->getResourceManager().getModelCopy(animName, shader));
		animationEntity5->getComponent<ModelComponent>()->getModel()->setIsAnimated(true);
		animationEntity5->addComponent<AnimationComponent>(&app->getResourceManager().getAnimationStack(animName));
		animationEntity5->getComponent<AnimationComponent>()->currentAnimation = animationEntity5->getComponent<AnimationComponent>()->getAnimationStack()->getAnimation(i);

	}

	animName = "AnimationTest/BaseMesh_Anim.fbx";
	for (int i = 0; i < 1; i++) {
		auto animationEntity5 = ECS::Instance()->createEntity("BaseMesh_Anim-" + std::to_string(i));
		animationEntity5->addComponent<TransformComponent>();
		animationEntity5->getComponent<TransformComponent>()->translate(-1.0f - (i * 2), 1, 0);
		animationEntity5->getComponent<TransformComponent>()->rotateAroundX(-3.14f * 0.5f);
		animationEntity5->getComponent<TransformComponent>()->scale(0.01f);
		animationEntity5->addComponent<ModelComponent>(&app->getResourceManager().getModelCopy(animName, shader));
		animationEntity5->getComponent<ModelComponent>()->getModel()->setIsAnimated(true);
		animationEntity5->addComponent<AnimationComponent>(&app->getResourceManager().getAnimationStack(animName));
		animationEntity5->getComponent<AnimationComponent>()->currentAnimation = animationEntity5->getComponent<AnimationComponent>()->getAnimationStack()->getAnimation(i);
	}
#endif
}
