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

	m_player->addComponent<AudioComponent>();
}

PlayerController::~PlayerController() {
	delete m_cam;
	m_projectiles.clear();
}

void PlayerController::setStartPosition(const glm::vec3& pos) {

}

CameraController* PlayerController::getCameraController() const {
	return m_cam;
}

void PlayerController::update(float dt) {
	// This can be removed once Input::WasKeyJustPressed is fixed since this only limit player from picking up candle right after putting it down.
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
