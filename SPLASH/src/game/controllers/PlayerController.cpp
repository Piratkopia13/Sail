#include "pch.h"
#include "PlayerController.h"
#include "Sail.h"

PlayerController::PlayerController(Camera* cam, Scene* scene) {
	m_cam = SAIL_NEW CameraController(cam);
	m_scene = scene;
	m_player = ECS::Instance()->createEntity("player_entity");

	//m_player->addComponent<MovementComponent>(/*initialSpeed*/ 0.f, /*initialDirection*/ m_cam->getCameraDirection());
	m_player->addComponent<TransformComponent>(m_cam->getCameraPosition());
	m_player->getComponent<TransformComponent>()->setStartTranslation(glm::vec3(0.0f, 3.f, 0.f));

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

// To be run at the beginning of each update tick
void PlayerController::prepareUpdate() {
	TransformComponent* transform = m_player->getComponent<TransformComponent>();
	if (transform) { transform->prepareUpdate(); }
}

void PlayerController::processKeyboardInput(float dt) {
	float speedModifier = 1.f;
	float forwardMovement = 0.0f;
	float rightMovement = 0.0f;
	float upMovement = 0.0f;

	PhysicsComponent* physicsComp = m_player->getComponent<PhysicsComponent>();

	float tempY = physicsComp->velocity.y;

	// Increase speed if shift or right trigger is pressed
	if (Input::IsKeyPressed(SAIL_KEY_SHIFT)) { speedModifier = RUN_SPEED; }

	if (Input::IsKeyPressed(SAIL_KEY_W)) { forwardMovement += 1.0f; }
	if (Input::IsKeyPressed(SAIL_KEY_S)) { forwardMovement -= 1.0f; }
	if (Input::IsKeyPressed(SAIL_KEY_A)) { rightMovement -= 1.0f; }
	if (Input::IsKeyPressed(SAIL_KEY_D)) { rightMovement += 1.0f; }
	if (Input::IsKeyPressed(SAIL_KEY_SPACE)) { 
		if (!m_wasSpacePressed) {
			tempY = 15.0f;
		}
		m_wasSpacePressed = true;
	}
	else {
		m_wasSpacePressed = false;
	}
	//if (Input::IsKeyPressed(SAIL_KEY_CONTROL)) { upMovement -= 1.0f; }


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

	// the player's transform will be modified in the physicSystem so save the player's current
	// position for interpolation later.
	playerTrans->prepareUpdate();

	// Prevent division by zero
	if (forwardMovement != 0.0f || rightMovement != 0.0f || upMovement != 0.0f) {
		// Calculate total movement
		physicsComp->velocity =
			glm::normalize(right * rightMovement + forward * forwardMovement)
			* m_movementSpeed * speedModifier;
	}
	else {
		physicsComp->velocity = glm::vec3(0.0f);
	}

	physicsComp->velocity.y = tempY;

	// Shooting

	// Shoot gun
	// TODO: This should probably be moved elsewhere.
	//       See if it should be done every tick or every frame and where the projectiles are to be created
	if (Input::IsMouseButtonPressed(0)) {
		if (m_projectileSpawnCounter == 0.f) {

			// Create projectile entity and attaching components
			auto e = ECS::Instance()->createEntity("new cube");
			e->addComponent<ModelComponent>(m_projectileModel);
			glm::vec3 camRight = glm::cross(m_cam->getCameraUp(), m_cam->getCameraDirection());
			e->addComponent<TransformComponent>(m_cam->getCameraPosition() + (m_cam->getCameraDirection() + camRight - m_cam->getCameraUp()), glm::vec3(0.f), glm::vec3(0.3f));
			e->addComponent<PhysicsComponent>();
			e->addComponent<BoundingBoxComponent>(m_projectileWireframeModel);
			e->getComponent<PhysicsComponent>()->velocity = m_cam->getCameraDirection() * 20.f;
			e->getComponent<PhysicsComponent>()->acceleration = glm::vec3(0.f, -25.f, 0.f);

			// Adding projectile to projectile vector to keep track of current projectiles
			Projectile proj;
			proj.projectile = e;
			m_projectiles.push_back(proj);

			// Add entity to scene for rendering, will most likely be changed once scene system is created
			m_scene->addEntity(e);

			m_projectileSpawnCounter += TIMESTEP;
		}
		else {
			m_projectileSpawnCounter += TIMESTEP;
			if (m_projectileSpawnCounter > 0.05f) {
				m_projectileSpawnCounter = 0.f;
			}
		}
	}
	else {
		m_projectileSpawnCounter = 0.f;
	}

	// Update for all projectiles
	//for (int i = 0; i < m_projectiles.size(); i++) {
	for (Projectile& p : m_projectiles) {
		p.lifeTime += TIMESTEP;
		if (p.lifeTime > 2.f) {
			ECS::Instance()->queueDestructionOfEntity(p.projectile);
		}
	}

	////moves the candlemodel and its pointlight to the correct position and rotates it to not spin when the player turns
	//forward = m_cam->getCameraDirection();
	//forward.y = 0.f;
	//forward = glm::normalize(forward);

	//right = glm::cross(glm::vec3(0.f, 1.f, 0.f), forward);
	//right = glm::normalize(right);
	//m_candle->getComponent<TransformComponent>()->prepareUpdate();
	//glm::vec3 playerToCandle = forward - right;
	//glm::vec3 candlePos = m_player->getComponent<TransformComponent>()->getTranslation() 
	//	+ playerToCandle - glm::vec3(0, 3.5f, 0);
	//m_candle->getComponent<TransformComponent>()->setTranslation(candlePos);
	//glm::vec3 candleRot = glm::vec3(0.f, glm::radians(-m_yaw), 0.f);
	//m_candle->getComponent<TransformComponent>()->setRotations(candleRot);
	//glm::vec3 flamePos = candlePos + glm::vec3(0, 3.22f, 0);
	//glm::vec3 plPos = flamePos - playerToCandle * 0.1f;
	//m_candle->getComponent<LightComponent>()->m_pointLight.setPosition(plPos);

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


	m_cam->setCameraPosition(playerTrans->getInterpolatedTranslation(alpha));
	m_cam->setCameraDirection(forwards);

	//moves the candlemodel and its pointlight to the correct position and rotates it to not spin when the player turns
	glm::vec3 forward = m_cam->getCameraDirection();
	forward.y = 0.f;
	forward = glm::normalize(forward);

	glm::vec3 right = glm::cross(glm::vec3(0.f, 1.f, 0.f), forward);
	right = glm::normalize(right);
	glm::vec3 playerToCandle = glm::vec3((forward - right)*0.2f);
	glm::vec3 candlePos = m_cam->getCameraPosition() + playerToCandle - glm::vec3(0, 0.35f, 0);
	m_candle->getComponent<TransformComponent>()->setTranslation(candlePos);
	glm::vec3 candleRot = glm::vec3(0.f, glm::radians(-m_yaw), 0.f);
	m_candle->getComponent<TransformComponent>()->setRotations(candleRot);
	m_candle->getComponent<TransformComponent>()->prepareUpdate();
	glm::vec3 flamePos = candlePos + glm::vec3(0, 0.37f, 0);
	glm::vec3 plPos = flamePos - playerToCandle * 0.1f;
	m_candle->getComponent<LightComponent>()->m_pointLight.setPosition(plPos);
}

void PlayerController::destroyOldProjectiles() {
	// Remove old projectiles
	for (int i = 0; i < m_projectiles.size(); i++) {
		if (m_projectiles[i].projectile->isAboutToBeDestroyed()) {
			ECS::Instance()->destroyEntity(m_projectiles[i].projectile);
			m_projectiles.erase(m_projectiles.begin() + i);
			i--;
		}
	}
}

// NOTE: Keyboard and mouse input processing has been moved to their own functions above this one
void PlayerController::update(float dt) {
	for (int i = 0; i < m_projectiles.size(); i++) {
		for (unsigned int j = 0; j < m_candles->size(); j++) {
			auto collisions = m_projectiles[i].projectile->getComponent<PhysicsComponent>()->collisions;
			for (unsigned int k = 0; k < collisions.size(); k++) {
				if (collisions[k].entity == m_candles->at(j).get()) {
					m_candles->at(j)->removeComponent<LightComponent>();
				}
			}
		}
	}
}

std::shared_ptr<Entity> PlayerController::getEntity() {
	return m_player;
}

void PlayerController::setProjectileModels(Model* model, Model* wireframeModel) {
	m_projectileModel = model;
	m_projectileWireframeModel = wireframeModel;
}

void PlayerController::provideCandles(std::vector<Entity::SPtr>* candles) {
	m_candles = candles;
}

std::shared_ptr<Entity> PlayerController::getCandle() {
	return m_candle;
}

//creates and binds the candle model and a pointlight for the player.
void PlayerController::createCandle(Model* model) {
	auto e = ECS::Instance()->createEntity("PlayerCandle");//;//ECS::Instance()->createEntity("PlayerCandle");
	e->addComponent<ModelComponent>(model);
	glm::vec3 camRight = glm::cross(m_cam->getCameraUp(), m_cam->getCameraDirection());
	//camRight = glm::normalize(camRight);
	glm::vec3 candlePos = -m_cam->getCameraDirection() + camRight;// -m_cam->getCameraUp();
	e->addComponent<TransformComponent>(candlePos);// , m_player->getComponent<TransformComponent>());
	//e->addComponent<TransformComponent>(glm::vec3(-1.f, -3.f, 1.f), m_player->getComponent<TransformComponent>());
	//e->getComponent<TransformComponent>()->setParent(m_player->getComponent<TransformComponent>());
	m_candle = e;
	PointLight pl;
	glm::vec3 lightPos = e->getComponent<TransformComponent>()->getTranslation();
	pl.setColor(glm::vec3(0.5f, 0.5f, 0.5f));
	pl.setPosition(glm::vec3(lightPos.x, lightPos.y + 3.1f, lightPos.z));
	pl.setAttenuation(.0f, 0.1f, 0.02f);
	pl.setIndex(2);
	e->addComponent<LightComponent>(pl);
	m_scene->setPlayerCandle(e);

}
