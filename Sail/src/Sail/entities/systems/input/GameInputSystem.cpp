#include "pch.h"
#include "GameInputSystem.h"
#include "Sail/graphics/camera/CameraController.h"
#include "Sail/entities/components/Components.h"
#include "../../../api/Input.h"
#include "Sail/KeyBinds.h"
#include "Sail/Application.h"
#include "Sail/utils/GameDataTracker.h"
#include "../../ECS.h"
#include "../physics/UpdateBoundingBoxSystem.h"


GameInputSystem::GameInputSystem() : BaseComponentSystem() {
	// TODO: System owner should check if this is correct
	registerComponent<PlayerComponent>(true, true, false);
	registerComponent<MovementComponent>(true, true, true);
	registerComponent<SpeedLimitComponent>(true, true, false);
	registerComponent<CollisionComponent>(true, true, false);
	registerComponent<AudioComponent>(true, true, true);
	
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

void GameInputSystem::stop() {
	clean();
}

void GameInputSystem::processKeyboardInput(const float& dt) {
	for (auto e : entities) {
		MovementComponent* movement = e->getComponent<MovementComponent>();
		CollisionComponent* collision = e->getComponent<CollisionComponent>();
		SpeedLimitComponent* speedLimit = e->getComponent<SpeedLimitComponent>();
		AudioComponent* audio = e->getComponent<AudioComponent>();

		// Get player movement inputs
		Movement playerMovement = getPlayerMovementInput(e);

		// Player puts down candle
#ifndef _DEBUG
		if (Input::WasKeyJustPressed(KeyBinds::putDownCandle)){
			putDownCandle(e);
		}
#endif

		// Calculate forward vector for player
		glm::vec3 forward = m_cam->getCameraDirection();
		forward.y = 0.f;
		forward = glm::normalize(forward);

		// Calculate right vector for player
		glm::vec3 right = glm::cross(glm::vec3(0.f, 1.f, 0.f), forward);
		right = glm::normalize(right);

		// Prevent division by zero
		if (playerMovement.forwardMovement != 0.0f || playerMovement.rightMovement != 0.0f) {
			// Calculate total movement
			float acceleration = 70.0f - (glm::length(movement->velocity) / speedLimit->maxSpeed) * 20.0f;
			if (!collision->onGround) {
				acceleration = acceleration * 0.5f;
				// AUDIO TESTING (turn OFF looping running sound)
				audio->m_isPlaying[SoundType::RUN] = false;
			}
			// AUDIO TESTING (playing a looping running sound)
			else if (m_runSoundTimer > 0.3f) {
				audio->m_isPlaying[SoundType::RUN] = true;
			}
			else {
				m_runSoundTimer += dt;
			}

			movement->accelerationToAdd =
				glm::normalize(right * playerMovement.rightMovement + forward * playerMovement.forwardMovement)
				* acceleration;
		}
		else {
			// AUDIO TESTING (turn OFF looping running sound)
			audio->m_isPlaying[SoundType::RUN] = false;
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
			glm::vec3 gunPosition = m_cam->getCameraPosition() +(m_cam->getCameraDirection() + camRight - m_cam->getCameraUp());
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

		m_cam->setCameraPosition(glm::vec3(playerTrans->getInterpolatedTranslation(alpha) + glm::vec3(0.f, playerBB->getBoundingBox()->getHalfSize().y * 1.8f, 0.f)));
		m_cam->setCameraDirection(forwards);
	}
}

void GameInputSystem::putDownCandle(Entity* e) {
	for (int i = 0; i < e->getChildEntities().size(); i++) {
		auto candleE = e->getChildEntities()[i];
		auto candleComp = candleE->getComponent<CandleComponent>();

		auto candleTransComp = candleE->getComponent<TransformComponent>();
		CollisionComponent* collision = e->getComponent<CollisionComponent>();
		auto playerTransComp = e->getComponent<TransformComponent>();
		if (candleComp->isCarried() && collision->onGround) {
			candleComp->toggleCarried();

			candleTransComp->removeParent();
			candleTransComp->setTranslation(playerTransComp->getTranslation() + glm::vec3(m_cam->getCameraDirection().x, 0.0f, m_cam->getCameraDirection().z));
			ECS::Instance()->getSystem<UpdateBoundingBoxSystem>()->update(0.0f);
			i = e->getChildEntities().size();
		}
		else if (!candleComp->isCarried() && glm::length(playerTransComp->getTranslation() - candleTransComp->getTranslation()) < 2.0f) {
			candleComp->toggleCarried();
			candleTransComp->setTranslation(glm::vec3(0.f, 2.0f, 0.f));
			candleTransComp->setParent(playerTransComp);
			i = e->getChildEntities().size();
		}
	}
}

Movement GameInputSystem::getPlayerMovementInput(Entity* e) {
	Movement playerMovement;
	
	if (Input::IsKeyPressed(KeyBinds::sprint)) { playerMovement.speedModifier = m_runSpeed; }
	if (Input::IsKeyPressed(KeyBinds::moveForward)) { playerMovement.forwardMovement += 1.0f; }
	if (Input::IsKeyPressed(KeyBinds::moveBackward)) { playerMovement.forwardMovement -= 1.0f; }
	if (Input::IsKeyPressed(KeyBinds::moveLeft)) { playerMovement.rightMovement -= 1.0f; }
	if (Input::IsKeyPressed(KeyBinds::moveRight)) { playerMovement.rightMovement += 1.0f; }
	if (Input::IsKeyPressed(KeyBinds::moveUp)) {
		if (!m_wasSpacePressed && e->getComponent<CollisionComponent>()->onGround) {
			e->getComponent<MovementComponent>()->velocity.y = 5.0f;
			// AUDIO TESTING - JUMPING
			e->getComponent<AudioComponent>()->m_isPlaying[SoundType::JUMP] = true;
			m_gameDataTracker->logJump();
		}
		m_wasSpacePressed = true;
	}
	else {
		m_wasSpacePressed = false;
	}

	return playerMovement;
}