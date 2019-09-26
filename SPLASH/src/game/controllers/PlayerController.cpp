#include "pch.h"
#include "PlayerController.h"
#include "Sail.h"
#include "Sail/TimeSettings.h"


PlayerController::PlayerController(Camera* cam, Scene* scene) {
	m_cam = SAIL_NEW CameraController(cam);
	m_scene = scene;
	m_player = ECS::Instance()->createEntity("player_entity");


	m_player->addComponent<TransformComponent>(m_cam->getCameraPosition());
	m_player->getComponent<TransformComponent>()->setStartTranslation(glm::vec3(0.0f, 0.f, 0.f));
	m_yaw = 90.f;
	m_pitch = 0.f;
	m_roll = 0.f;
}

PlayerController::~PlayerController() {
	delete m_cam;
	m_projectiles.clear();
}

void PlayerController::setStartPosition(const glm::vec3& pos) {

}


void PlayerController::processKeyboardInput(float dt) {
	float speedModifier = 1.f;
	float forwardMovement = 0.0f;
	float rightMovement = 0.0f;
	float upMovement = 0.0f;

	PhysicsComponent* physicsComp = m_player->getComponent<PhysicsComponent>();

	// Increase speed if shift or right trigger is pressed
	if (Input::IsKeyPressed(KeyBinds::sprint)) { speedModifier = RUN_SPEED; }

	if (Input::IsKeyPressed(KeyBinds::moveForward)) { forwardMovement += 1.0f; }
	if (Input::IsKeyPressed(KeyBinds::moveBackward)) { forwardMovement -= 1.0f; }
	if (Input::IsKeyPressed(KeyBinds::moveLeft)) { rightMovement -= 1.0f; }
	if (Input::IsKeyPressed(KeyBinds::moveRight)) { rightMovement += 1.0f; }
	if (Input::IsKeyPressed(KeyBinds::moveUp)) {
		if (!m_wasSpacePressed) {
			physicsComp->velocity.y = 5.0f;
		}
		m_wasSpacePressed = true;
	}
	else {
		m_wasSpacePressed = false;
	}

	// TODO:: Fix Input::WasKeyJustPressed
	if (Input::IsKeyPressed(KeyBinds::putDownCandle) && m_canPickUp) {
		for (int i = 0; i < m_player->getChildEntities().size(); i++){
			auto e = m_player->getChildEntities()[i];
			auto candle = e->getComponent<CandleComponent>();

			auto cTransComp = e->getComponent<TransformComponent>();
			auto pTransComp = m_player->getComponent<TransformComponent>();
			if ( candle->isCarried() && physicsComp->onGround) {
				candle->putDown();

				cTransComp->setTranslation(pTransComp->getTranslation() + glm::vec3(m_cam->getCameraDirection().x, -0.9f, m_cam->getCameraDirection().z));
				cTransComp->removeParent();
				i = m_player->getChildEntities().size();
			}
			else if (!candle->isCarried() && glm::length(pTransComp->getTranslation() - cTransComp->getTranslation()) < 2.0f) {
				candle->pickUp();
				cTransComp->setTranslation(glm::vec3(0.f, 1.1f, 0.f));
				cTransComp->setParent(pTransComp);
				i = m_player->getChildEntities().size();
			}
		}
		m_canPickUp = false;
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

	TransformComponent* playerTrans = m_player->getComponent<TransformComponent>();

	// Prevent division by zero
	if (forwardMovement != 0.0f || rightMovement != 0.0f) {
		// Calculate total movement
		float acceleration = 70.0f - (glm::length(physicsComp->velocity) / physicsComp->maxSpeed) * 20.0f;
		if (!physicsComp->onGround) {
			acceleration = acceleration * 0.5f;
		}
		physicsComp->accelerationToAdd += 
			glm::normalize(right * rightMovement + forward * forwardMovement)
			* acceleration;
	}

	// Shooting

	// Shoot gun
	// TODO: This should probably be moved elsewhere.
	//       See if it should be done every tick or every frame and where the projectiles are to be created
	if (Input::IsMouseButtonPressed(0)) {
		// TODO: add and tweak guncomponent+projectile system once playercontroller is changed to a system
		glm::vec3 camRight = glm::cross(m_cam->getCameraUp(), m_cam->getCameraDirection());
		glm::vec3 gunPosition = m_cam->getCameraPosition() + (m_cam->getCameraDirection() + camRight - m_cam->getCameraUp());
		m_player->getComponent<GunComponent>()->setFiring(gunPosition, m_cam->getCameraDirection());
	}
}

void PlayerController::processMouseInput(float dt) {
	PhysicsComponent* physicsComp = m_player->getComponent<PhysicsComponent>();

	// Mouse input

	// Toggle cursor capture on right click
	if (Input::WasMouseButtonJustPressed(SAIL_MOUSE_RIGHT_BUTTON)) {
		Input::HideCursor(!Input::IsCursorHidden());
	}

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

void PlayerController::updateCameraPosition(float alpha) {
	TransformComponent* playerTrans = m_player->getComponent<TransformComponent>();
	BoundingBoxComponent* playerBB = m_player->getComponent<BoundingBoxComponent>();

	glm::vec3 forwards(
		std::cos(glm::radians(m_pitch)) * std::cos(glm::radians(m_yaw)),
		std::sin(glm::radians(m_pitch)),
		std::cos(glm::radians(m_pitch)) * std::sin(glm::radians(m_yaw))
	);
	forwards = glm::normalize(forwards);
	//playerTrans->setForward(forwards); //needed?


	m_cam->setCameraPosition(glm::vec3(playerTrans->getInterpolatedTranslation(alpha) + glm::vec3(0.f,playerBB->getBoundingBox()->getHalfSize().y*0.8f,0.f)));
	m_cam->setCameraDirection(forwards);
}


CameraController* PlayerController::getCameraController() const {
	return m_cam;
}

void PlayerController::update(float dt) {
	if (!m_canPickUp) {
		m_candleTimer += dt;
		if (m_candleTimer > m_candleLimit) {
			m_candleTimer = 0.f;
			m_canPickUp = true;
		}
	}
}

std::shared_ptr<Entity> PlayerController::getEntity() {
	return m_player;
}

void PlayerController::setProjectileModels(Model* model, Model* wireframeModel) {
	m_projectileModel = model;
	m_projectileWireframeModel = wireframeModel;
	m_player->addComponent<GunComponent>(m_projectileModel);
}


float PlayerController::getYaw() const {
	return m_yaw;
}
