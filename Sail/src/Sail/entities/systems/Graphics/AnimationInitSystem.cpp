#include "pch.h"
#include "AnimationInitSystem.h"
#include "Sail/graphics/shader/dxr/GBufferOutShader.h"
#include "../../ECS.h"
#include "Sail/Application.h"
#include "../../components/TransformComponent.h"
#include "../../components/ModelComponent.h"
#include "../../components/AnimationComponent.h"
#include "../../components/NoEntityComponent.h"
#include "Sail/entities/EntityFactory.hpp"
#include "Sail/graphics/shader/material/WireframeShader.h"
#include "Sail/entities/components/CandleComponent.h"
#include "Sail/entities/components/BoundingBoxComponent.h"
#include "Sail/entities/components/CollidableComponent.h"
#include "Sail/entities/components/OnlineOwnerComponent.h"
#include "Sail/entities/components/CandleComponent.h"
#include "Sail/entities/components/LightComponent.h"
#include "Sail/graphics/light/PointLight.h"

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
	std::string name = "DocTorch.fbx";

	auto* wireframeShader = &Application::getInstance()->getResourceManager().getShaderSet<WireframeShader>();
	Model* lightModel = &Application::getInstance()->getResourceManager().getModel("candleExported.fbx", shader);
	lightModel->getMesh(0)->getMaterial()->setAlbedoTexture("sponza/textures/candleBasicTexture.tga");
	//Wireframe bounding box model
	Model* boundingBoxModel = &Application::getInstance()->getResourceManager().getModel("boundingBox.fbx", wireframeShader);
	boundingBoxModel->getMesh(0)->getMaterial()->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));



	AnimationStack* stack = &app->getResourceManager().getAnimationStack(name);
	unsigned int animationCount = stack->getAnimationCount();
	for (unsigned int i = 0; i < animationCount; i++) {
		auto animationEntity2 = ECS::Instance()->createEntity("Doc" + std::to_string(i));
		EntityFactory::CreateGenericPlayer(animationEntity2, i, glm::vec3(-2.0f + i * 1.2f, 0, -2));

		AnimationComponent* ac = animationEntity2->addComponent<AnimationComponent>(stack);
		ac->setAnimation(i);
	}

}
