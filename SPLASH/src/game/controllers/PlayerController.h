#pragma once

class CameraController;
class Camera;
class Entity;

constexpr float RUN_SPEED = 2.0f;

class Scene;
class Model;

// Will most likely be obselete once collision is implemented and projectiles can be destroyed that way.
struct Projectile {
	std::shared_ptr<Entity> projectile;
	float lifeTime = 0.f;
};

class PlayerController {
public:
	PlayerController(Camera* cam, Scene* scene);
	~PlayerController();

	void update(float dt);

	void setStartPosition(const glm::vec3& pos);
	void prepareUpdate();

	void processKeyboardInput(float dt);
	void processMouseInput(float dt);

	std::shared_ptr<Entity> getEntity();

	void setProjectileModel(Model* model);


	// TEMPORARY SOLUTION, should be done in a more threadsafe way
	// Call at the start of the render loop and nowhere else
	void destroyOldProjectiles();
private:
	float m_movementSpeed = 20.f;


	// "Attached" camera
	CameraController* m_cam;
	
	Scene* m_scene;

	Model* m_projectileModel;
	
	// Never access without the mutex lock
	std::vector<Projectile> m_projectiles;
	std::mutex m_projectileLock;


	std::shared_ptr<Entity> m_player;

	// #netcodeNote not thread safe, might cause issues
	float m_yaw, m_pitch, m_roll;

	float m_lookSensitivityMouse = 0.1f;

	float m_projectileSpawnCounter = 0.f;

};