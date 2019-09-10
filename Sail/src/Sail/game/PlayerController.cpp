#include "pch.h"
#include "PlayerController.h"
#include "../Application.h"
#include "../KeyCodes.h"
#include "../MouseButtonCodes.h"
#include "../graphics/camera/CameraController.h"
#include "../entities/Entity.h"
#include "../entities/components/MovementComponent.h"
#include "../entities/components/TransformComponent.h"

PlayerController::PlayerController(Camera* cam) {
	m_cam = new CameraController(cam);
	m_player = Entity::Create("player_entity");
	m_player->addComponent<MovementComponent>(/*initialSpeed*/ 0.f, /*initialDirection*/ m_cam->getCameraDirection());
	m_player->addComponent<TransformComponent>(m_cam->getCameraPosition());

	m_yaw = 90.f;
	m_pitch = 0.f;
	m_roll = 0.f;
}

PlayerController::~PlayerController() {}

void PlayerController::update(float dt) {
	float speedModifier = 1.f;
	float movementSpeed = 0.f;
	float lookSensitivityMouse = 0.1f;
	float lookSensitivityController = 90.0f * 0.016f;

	float forwardM = 0.f, backM = 0.f, leftM = 0.f, rightM = 0.f, upM = 0.f, downM = 0.f;

	MovementComponent* playerMovComp = m_player->getComponent<MovementComponent>();

	// Increase speed if shift or right trigger is pressed
	if ( Input::IsKeyPressed(SAIL_KEY_SHIFT) ) {
		speedModifier = 5.f;
	}

	//
	// Forwards / backwards motion
	//

	// Keyboard
	if ( Input::IsKeyPressed(SAIL_KEY_W) ) {
		playerMovComp->setSpeed(m_movementSpeed * speedModifier);
		forwardM = 1.0f;
	}

	if ( Input::IsKeyPressed(SAIL_KEY_S) ) {
		playerMovComp->setSpeed(m_movementSpeed * speedModifier);
		backM = 1.0f;
	}

	//
	// Side to side motion
	//

	glm::vec3 right = glm::cross(glm::vec3(0.f, 1.f, 0.f), m_cam->getCameraDirection());
	right = glm::normalize(right);
	// Gamepad
	//setCameraPosition(getCameraPosition() - right * gpState.thumbSticks.leftX * movementSpeed);

	// Keyboard
	if ( Input::IsKeyPressed(SAIL_KEY_A) ) {
		playerMovComp->setSpeed(m_movementSpeed * speedModifier);
		leftM = 1.0f;
	}
	if ( Input::IsKeyPressed(SAIL_KEY_D) ) {
		playerMovComp->setSpeed(m_movementSpeed * speedModifier);
		rightM = 1.0f;
	}

	//
	// Up and down motion
	//

	// Keyboard
	if ( Input::IsKeyPressed(SAIL_KEY_SPACE) ) {
		playerMovComp->setSpeed(m_movementSpeed * speedModifier);
		upM = 1.0f;
	}
	if ( Input::IsKeyPressed(SAIL_KEY_CONTROL) ) {
		playerMovComp->setSpeed(m_movementSpeed * speedModifier);
		downM = 1.0f;
	}

	//
	// Look around motion
	//

	// Mouse input

	// Toggle cursor capture on right click
	if ( Input::WasMouseButtonJustPressed(SAIL_MOUSE_RIGHT_BUTTON) ) {
		Input::HideCursor(!Input::IsCursorHidden());
	}

	if ( Input::IsCursorHidden() ) {
		glm::ivec2& mouseDelta = Input::GetMouseDelta();
		m_pitch -= mouseDelta.y * lookSensitivityMouse;
		m_yaw -= mouseDelta.x * lookSensitivityMouse;
	}


	// Lock pitch to the range -89 - 89
	if ( m_pitch >= 89 ) {
		m_pitch = 89;
	}
	else if ( m_pitch <= -89 ) {
		m_pitch = -89;
	}

	// Lock yaw to the range 0 - 360
	if ( m_yaw >= 360 ) {
		m_yaw -= 360;
	}
	else if ( m_yaw <= 0 ) {
		m_yaw += 360;
	}

	glm::vec3 forwards(
		std::cos(glm::radians(m_pitch)) * std::cos(glm::radians(m_yaw)),
		std::sin(glm::radians(m_pitch)),
		std::cos(glm::radians(m_pitch)) * std::sin(glm::radians(m_yaw))
	);
	forwards = glm::normalize(forwards);

	glm::vec3 forward = m_cam->getCameraDirection();
	float totM = forwardM + backM + rightM + leftM + upM + downM;
	//Logger::Warning(std::to_string(totM));
	if ( totM != 0.f ) {
		glm::vec3 dir = (forward * forwardM) - (forward * backM) + (right * rightM) - (right * leftM)
			/*Only for flying*/ + ( m_cam->getCameraUp() * upM ) - ( m_cam->getCameraUp() * downM );

		playerMovComp->setDirection(glm::normalize(dir));
	}

	TransformComponent* playerTrans = m_player->getComponent<TransformComponent>();
	m_cam->setCameraPosition(playerTrans->getTranslation());
	// TODO: Replace with transform rotation/direction
	/*Logger::Warning("totM: " + std::to_string(totM) + 
					" fowards: " + std::to_string(forwards.x) + 
					" " + std::to_string(forwards.y)
					+ " " + std::to_string(forwards.z));*/
	m_cam->setCameraDirection(forwards);
}

std::shared_ptr<Entity> PlayerController::getEntity() {
	return m_player;
}
