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
	//registerComponent<BoundingBoxComponent>(true, true, false);
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


		//glm::vec3 forwards(
		//	std::cos(glm::radians(m_pitch)) * std::cos(glm::radians(m_yaw + 90)),
		//	std::sin(glm::radians(m_pitch)),
		//	std::cos(glm::radians(m_pitch)) * std::sin(glm::radians(m_yaw + 90))
		//);
		//forwards = glm::normalize(forwards);

		//playerTrans->setRotations(0.f, glm::radians(-m_yaw), 0.f);

		//m_cam->setCameraPosition(glm::vec3(playerTrans->getInterpolatedTranslation(alpha) + glm::vec3(0.f, playerBB->getBoundingBox()->getHalfSize().y * 1.8f, 0.f)));

		//// TODO: use the other way we set the camera position
		//m_cam->setCameraDirection(glm::vec3(-playerTrans->getForward()));








		// TODO: interpolation
		//const glm::vec3 finalPos = transform->getRenderMatrix(alpha) * glm::vec4(animation->headPositionLocalCurrent, 1.f);

		//m_cam->setCameraPosition(finalPos);


		//const float cosRadPitch = std::cosf(glm::radians(-animation->pitch));
		//const float sinRadPitch = std::sinf(glm::radians(-animation->pitch));

		//float yaw = glm::eulerAngles(transform->getInterpolatedRotation(alpha)).y;

		//const float cosRadYaw = std::cosf(glm::radians(-yaw + 90));
		//const float sinRadYaw = std::sinf(glm::radians(-yaw + 90));

		//const glm::vec3 forwards = glm::normalize(glm::vec3(
		//	cosRadPitch * cosRadYaw,
		//	sinRadPitch,
		//	cosRadPitch * sinRadYaw));

		////const glm::quat rotated = transform->getInterpolatedRotation(alpha) * glm::quat(glm::vec3(0.f, -animation->pitch, 0.f));

		////const glm::vec3 forwards = rotated * glm::vec3(0.f, 0.f, 1.f);

		//m_cam->setCameraDirection(forwards);







		//const float yaw = -glm::degrees(transform->getInterpolatedRotation(alpha).y);
		const float pitch = -glm::degrees(interpolate(animation->prevPitch, animation->pitch, alpha));


		const glm::vec3 finalPos = transform->getRenderMatrix(alpha) * glm::vec4(animation->headPositionLocalCurrent, 1.f);
		const glm::vec3 camPos = glm::vec3(finalPos);

		m_cam->setCameraPosition(camPos);


		const float cosRadPitch = std::cosf(glm::radians(pitch));
		const float sinRadPitch = std::sinf(glm::radians(pitch));
		const float cosRadYaw = std::cosf(glm::radians(yaw + 90));
		const float sinRadYaw = std::sinf(glm::radians(yaw + 90));

		const glm::vec3 forwards = glm::normalize(glm::vec3(
			cosRadPitch * cosRadYaw,
			sinRadPitch,
			cosRadPitch * sinRadYaw));

		m_cam->setCameraDirection(forwards);
	}
}


