#include "pch.h"
#include "GameInputSystem.h"
#include "Sail/graphics/camera/CameraController.h"
#include "Sail/entities/components/Components.h"
#include "../../../api/Input.h"
#include "Sail/KeyBinds.h"
#include "Sail/Application.h"
#include "Sail/utils/GameDataTracker.h"

GameInputSystem::GameInputSystem() : BaseComponentSystem() {
	// TODO: System owner should check if this is correct
	registerComponent<PlayerComponent>(true, true, false);
	
	// cam variables
	m_yaw = 90.f;
	m_pitch = 0.f;
	m_roll = 0.f;

	m_gameDataTracker = &GameDataTracker::getInstance();
}

GameInputSystem::~GameInputSystem() {
	clean();
}


void GameInputSystem::update(float dt, float alpha) {
	this->processKeyboardInput(dt);
	this->processMouseInput(dt);
	this->updateCameraPosition(alpha);
}

void GameInputSystem::initialize(Camera* cam) {
	if (m_cam == nullptr) {
		m_cam = SAIL_NEW CameraController(cam);
	}
}

void GameInputSystem::clean() { 
	Memory::SafeDelete(m_cam);
}

void GameInputSystem::processPerFrameInput() {

}

void GameInputSystem::processPerTickInput() {

}

void GameInputSystem::processKeyboardInput(const float& dt) {

	float speedModifier = 1.f;
	float forwardMovement = 0.0f;
	float rightMovement = 0.0f;
	float upMovement = 0.0f;
	
	for (auto e : entities) {
		PhysicsComponent* physicsComp = e->getComponent<PhysicsComponent>();
		AudioComponent* audioComp = e->getComponent<AudioComponent>();

		// Increase speed if shift or right trigger is pressed
		if (Input::IsKeyPressed(KeyBinds::sprint)) { speedModifier = m_runSpeed; }

		if (Input::IsKeyPressed(KeyBinds::moveForward)) { forwardMovement += 1.0f; }
		if (Input::IsKeyPressed(KeyBinds::moveBackward)) { forwardMovement -= 1.0f; }
		if (Input::IsKeyPressed(KeyBinds::moveLeft)) { rightMovement -= 1.0f; }
		if (Input::IsKeyPressed(KeyBinds::moveRight)) { rightMovement += 1.0f; }
		if (Input::IsKeyPressed(KeyBinds::moveUp)) {
			if (!m_wasSpacePressed){// && physicsComp->onGround) {
				physicsComp->velocity.y = 5.0f;
				// AUDIO TESTING - JUMPING
				audioComp->m_isPlaying[SoundType::JUMP] = true;
				m_gameDataTracker->logJump();
			}
			m_wasSpacePressed = true;
		}
		else {
			m_wasSpacePressed = false;
		}

		if (Input::WasKeyJustPressed(KeyBinds::putDownCandle)){
			for (int i = 0; i < e->getChildEntities().size(); i++) {
				auto candleE = e->getChildEntities()[i];
				auto candleComp = candleE->getComponent<CandleComponent>();

				auto candleTransComp = candleE->getComponent<TransformComponent>();
				auto playerTransComp = e->getComponent<TransformComponent>();
				if (candleComp->isCarried() && physicsComp->onGround) {
					candleComp->toggleCarried();

					candleTransComp->setTranslation(playerTransComp->getTranslation() + glm::vec3(m_cam->getCameraDirection().x, -0.9f, m_cam->getCameraDirection().z));
					candleTransComp->removeParent();
					i = e->getChildEntities().size();
				}
				else if (!candleComp->isCarried() && glm::length(playerTransComp->getTranslation() - candleTransComp->getTranslation()) < 2.0f) {
					candleComp->toggleCarried();
					candleTransComp->setTranslation(glm::vec3(0.f, 1.1f, 0.f));
					candleTransComp->setParent(playerTransComp);
					i = e->getChildEntities().size();
				}
			}
		}

		glm::vec3 forwards(
			std::cos(glm::radians(m_pitch)) * std::cos(glm::radians(m_yaw)),
			std::sin(glm::radians(m_pitch)),
			std::cos(glm::radians(m_pitch)) * std::sin(glm::radians(m_yaw))
		);
		forwards = glm::normalize(forwards);

		glm::vec3 forward = m_cam->getCameraDirection();
		forward.y = 0.f;
		forward = glm::normalize(forward);

		glm::vec3 right = glm::cross(glm::vec3(0.f, 1.f, 0.f), forward);
		right = glm::normalize(right);

		// Prevent division by zero
		if (forwardMovement != 0.0f || rightMovement != 0.0f) {
			// Calculate total movement
			float acceleration = 70.0f - (glm::length(physicsComp->velocity) / physicsComp->maxSpeed) * 20.0f;
			if (!physicsComp->onGround) {
				acceleration = acceleration * 0.5f;
				// AUDIO TESTING (turn OFF looping running sound)
				audioComp->m_isPlaying[SoundType::RUN] = false;
			}
			// AUDIO TESTING (playing a looping running sound)
			else if (m_runSoundTimer > 0.3f) {
				audioComp->m_isPlaying[SoundType::RUN] = true;
			}
			else {
				m_runSoundTimer += dt;
			}

			physicsComp->accelerationToAdd = 
				glm::normalize(right * rightMovement + forward * forwardMovement)
				* acceleration;
		}
		else {
			// AUDIO TESTING (turn OFF looping running sound)
			audioComp->m_isPlaying[SoundType::RUN] = false;
			m_runSoundTimer = 0.0f;
		}
	}
}

void GameInputSystem::processMouseInput(const float& dt) {
	// Toggle cursor capture on right click
	for (auto e : entities) {
		AudioComponent* audioComp = e->getComponent<AudioComponent>();

		if (Input::WasMouseButtonJustPressed(KeyBinds::disableCursor)) {
			Input::HideCursor(!Input::IsCursorHidden());
		}

		if (Input::IsMouseButtonPressed(KeyBinds::shoot)) {
			glm::vec3 camRight = glm::cross(m_cam->getCameraUp(), m_cam->getCameraDirection());
			glm::vec3 gunPosition = m_cam->getCameraPosition() + (m_cam->getCameraDirection() + camRight - m_cam->getCameraUp());
			e->getComponent<GunComponent>()->setFiring(gunPosition, m_cam->getCameraDirection());
		}

		// Update pitch & yaw if window has focus
		if (Input::IsCursorHidden()) {
			glm::ivec2& mouseDelta = Input::GetMouseDelta();
			m_pitch -= mouseDelta.y * m_lookSensitivityMouse;
			m_yaw -= mouseDelta.x * m_lookSensitivityMouse;
		}

		// Lock pitch to the range -89 - 89
		if (m_pitch >= 89) {
			m_pitch = 89;
		}
		else if (m_pitch <= -89) {
			m_pitch = -89;
		}

		// Lock yaw to the range 0 - 360
		if (m_yaw >= 360) {
			m_yaw -= 360;
		}
		else if (m_yaw <= 0) {
			m_yaw += 360;
		}
	}
}

glm::vec3 GameInputSystem::calculateNormalizedRightVector() {
	// If u want to know how this works, ask gustav.
	glm::vec3 forwardVector = m_cam->getCameraDirection();
	forwardVector.y = 0.f;
	forwardVector = glm::normalize(forwardVector);

	glm::vec3 rightVector = glm::cross(glm::vec3(0.f, 1.f, 0.f), forwardVector);
	rightVector = glm::normalize(rightVector);

	return rightVector;
}

void GameInputSystem::updateCameraPosition(float alpha) {
	for (auto e : entities) {


		TransformComponent* playerTrans = e->getComponent<TransformComponent>();
		BoundingBoxComponent* playerBB = e->getComponent<BoundingBoxComponent>();

		glm::vec3 forwards(
			std::cos(glm::radians(m_pitch)) * std::cos(glm::radians(m_yaw)),
			std::sin(glm::radians(m_pitch)),
			std::cos(glm::radians(m_pitch)) * std::sin(glm::radians(m_yaw))
		);
		forwards = glm::normalize(forwards);

		m_cam->setCameraPosition(glm::vec3(playerTrans->getInterpolatedTranslation(alpha) + glm::vec3(0.f, playerBB->getBoundingBox()->getHalfSize().y * 0.8f, 0.f)));
		m_cam->setCameraDirection(forwards);
	}
}
