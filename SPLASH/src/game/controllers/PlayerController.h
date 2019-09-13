#pragma once

class CameraController;
class Camera;
class Entity;

// Stuff
class Projectile;
class Scene;
class Model;

class PlayerController {
public:
	PlayerController(Camera* cam, Scene* scene);
	~PlayerController();

	void update(float dt);

	//void setScene(Scene* scene);

	std::shared_ptr<Entity> getEntity();

	void setProjectileModel(Model* model);

private:
	float m_movementSpeed = 5.f;

	std::vector<Projectile*> m_projectiles;

	// "Attached" camera
	CameraController* m_cam;
	Scene* m_scene;

	Model* m_projectileModel;

	std::shared_ptr<Entity> m_player;

	float m_yaw, m_pitch, m_roll;

	float m_lookSensitivityMouse = 0.1f;

	float m_projectileSpawnCounter = 0.f;

};