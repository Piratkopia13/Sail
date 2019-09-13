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

void AiController::update() {
	if ( m_controlledEntity != nullptr ) {
		float speedModifier = 1.f;

		bool leftPressed = false, rightPressed = false, forwardPressed = true, backPressed = false, jumpPressed = true, 
			crouchPressed = false, speedModifierPressed = true;

		float forwardMovement = 0.0f;
		float rightMovement = 0.0f;
		float upMovement = 0.0f;

		PhysicsComponent* physicsComp = m_controlledEntity->getComponent<PhysicsComponent>();

		// Increase speed if shift or right trigger is pressed
		if ( speedModifierPressed ) {
			speedModifier = 5.f;
		}

		//
		// Forwards / backwards motion
		//

		// "Keyboard"
		if ( forwardPressed ) {
			forwardMovement += 1.0f;
		}

		if ( backPressed ) {
			forwardMovement -= 1.0f;
		}

		//
		// Side to side motion
		//

		// "Keyboard"
		if ( leftPressed ) {
			rightMovement -= 1.0f;
		}
		if ( rightPressed ) {
			rightMovement += 1.0f;
		}

		//
		// Up and down motion
		//

		// "Keyboard"
		if ( jumpPressed ) {
			upMovement += 1.0f;
		}
		if ( crouchPressed ) {
			upMovement -= 1.0f;
		}

		//
		// Look around motion
		//

		// "Mouse input"

		// Toggle cursor capture on right click
		physicsComp->rotation = glm::vec3(0.f, 100.f, 0.f);

		TransformComponent* eTC = m_controlledEntity->getComponent<TransformComponent>();

		float beginStuff = 5.f;
		float flyAway = 15.f;
		if ( eTC->getTranslation().y > beginStuff ) {
			physicsComp->rotation = glm::vec3(0.f, (( flyAway - eTC->getTranslation().y ) / beginStuff) * 100.f, 0.f);
		}
		if ( eTC->getTranslation().y > flyAway) {
			physicsComp->velocity.x += 3.f;
		}

		// Prevent division by zero
		if ( forwardMovement != 0.0f || rightMovement != 0.0f || upMovement != 0.0f ) {

			// Calculate total movement
			/*physicsComp->velocity =
				glm::normalize(eTC->getRight() * rightMovement + eTC->getForward() * forwardMovement + glm::vec3(0.0f, 1.0f, 0.0f) * upMovement)
				* speedModifier;*/
			physicsComp->velocity = glm::vec3(physicsComp->velocity.x, 1.f, physicsComp->velocity.z);
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
