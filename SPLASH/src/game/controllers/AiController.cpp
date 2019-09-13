#include "pch.h"
#include "AiController.h"
#include "Sail.h"

AiController::AiController() {
	m_controlledEntity = nullptr;

	m_yaw = 90.f;
	m_pitch = 0.f;
	m_roll = 0.f;
}

AiController::AiController(std::shared_ptr<Entity> toControl) 
	: AiController() 
{
	m_controlledEntity = toControl;
}

AiController::~AiController() {}

void AiController::update(float dt) {
	if ( m_controlledEntity != nullptr ) {
		float speedModifier = 1.f;

		bool aPressed = true, dPressed = false, wPressed = true, sPressed = false, spacePressed = false, ctrlPressed = false, shiftPressed = true;

		float forwardMovement = 0.0f;
		float rightMovement = 0.0f;
		float upMovement = 0.0f;

		PhysicsComponent* physicsComp = m_controlledEntity->getComponent<PhysicsComponent>();

		// Increase speed if shift or right trigger is pressed
		if ( shiftPressed ) {
			speedModifier = 5.f;
		}

		//
		// Forwards / backwards motion
		//

		// "Keyboard"
		if ( wPressed ) {
			forwardMovement += 1.0f;
		}

		if ( sPressed ) {
			forwardMovement -= 1.0f;
		}

		//
		// Side to side motion
		//

		// "Keyboard"
		if ( aPressed ) {
			rightMovement -= 1.0f;
		}
		if ( dPressed ) {
			rightMovement += 1.0f;
		}

		//
		// Up and down motion
		//

		// "Keyboard"
		if ( spacePressed ) {
			upMovement += 1.0f;
		}
		if ( ctrlPressed ) {
			upMovement -= 1.0f;
		}

		//
		// Look around motion
		//

		// "Mouse input"

		// Toggle cursor capture on right click
		physicsComp->rotation = glm::vec3(0.f, 1.f, 0.f);

		TransformComponent* eTC = m_controlledEntity->getComponent<TransformComponent>();

		// Prevent division by zero
		if ( forwardMovement != 0.0f || rightMovement != 0.0f || upMovement != 0.0f ) {

			// Calculate total movement
			physicsComp->velocity =
				glm::normalize(eTC->getRight() * rightMovement + eTC->getForward() * forwardMovement + glm::vec3(0.0f, 1.0f, 0.0f) * upMovement)
				* speedModifier;
		}
		else {
			physicsComp->velocity = glm::vec3(0.0f, 0.0f, 0.0f);
		}
	}
}

void AiController::setEntity(std::shared_ptr<Entity> toControl) {
	m_controlledEntity = toControl;
}

std::shared_ptr<Entity> AiController::getEntity() {
	return m_controlledEntity;
}
