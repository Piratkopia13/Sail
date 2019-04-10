#pragma once

#include <d3d11.h>
#include <glm/glm.hpp>

class Particle {
public:
	Particle(const glm::vec3& position, const glm::vec3& velocity, float scale = 1.0f, float gravityScale = 0.f, float lifetime = 10.f);
	~Particle();

	void update(float dt);
	const glm::vec3& getPosition() const;
	float getLifePercentage() const;
	bool isDead() const;

private:
	glm::vec3 m_position;
	glm::vec3 m_velocity;
	float m_gravityScale;
	float m_lifetime;
	float m_scale;
	float m_elapsedTime;
	bool m_alive;

};