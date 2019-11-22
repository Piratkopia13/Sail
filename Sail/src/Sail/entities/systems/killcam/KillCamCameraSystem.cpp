#include "pch.h"
#include "KillCamCameraSystem.h"

#include "Sail/graphics/camera/CameraController.h"
#include "Sail/utils/Utils.h"

#include "../../components/AnimationComponent.h"
//#include "../../components/BoundingBoxComponent.h"
#include "../../components/KillerComponent.h"
#include "../../components/TransformComponent.h"

KillCamCameraSystem::KillCamCameraSystem() {
	registerComponent<AnimationComponent>(true, true, false);
	registerComponent<KillerComponent>(true, false, false);
	registerComponent<TransformComponent>(true, true, true);
}

KillCamCameraSystem::~KillCamCameraSystem() {
}

void KillCamCameraSystem::stop() {
	Memory::SafeDelete(m_cam);
}

void KillCamCameraSystem::initialize(Camera* cam) {
	if (m_cam == nullptr) {
		m_cam = SAIL_NEW CameraController(cam);
	}
}

bool KillCamCameraSystem::onEvent(const Event& event) {
	return false;
}
void KillCamCameraSystem::update(float dt, float alpha) {
	updateCameraPosition(alpha);
}

void KillCamCameraSystem::updateCameraPosition(float alpha) {
	auto interpolate = [](float prev, float current, float alpha) {
		return (alpha * current) + ((1.0f - alpha) * prev);
	};

	for (auto e : entities) {
		AnimationComponent* animation = e->getComponent<AnimationComponent>();
		//BoundingBoxComponent* playerBB  = e->getComponent<BoundingBoxComponent>();
		TransformComponent* transform = e->getComponent<TransformComponent>();

		const glm::vec3 finalPos = transform->getRenderMatrix(alpha) * glm::vec4(animation->headPositionLocalCurrent, 1.f);
		const glm::vec3 camPos = glm::vec3(finalPos);

		m_cam->setCameraPosition(camPos);

		const glm::quat rot = glm::angleAxis(-interpolate(animation->prevPitch, animation->pitch, alpha), glm::vec3(1, 0, 0));
		const glm::quat rotated = glm::normalize(rot * transform->getInterpolatedRotation(alpha));

		const glm::vec3 forwards = rotated * glm::vec3(0.f, 0.f, 1.f);

		m_cam->setCameraDirection(forwards);

	}
}


