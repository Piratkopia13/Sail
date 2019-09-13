#pragma once

#include <memory>

class Entity;

class Projectile {
public:
	Projectile();
	~Projectile();

	void update(float dt);

private:
	float m_speed = 20.f;
	float m_lifeTime = 0.f;

	Model* m_model;

	std::shared_ptr<Entity> m_projectile;
};