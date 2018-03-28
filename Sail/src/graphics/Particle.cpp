#include "Particle.h"

using namespace DirectX;
using namespace SimpleMath;

Particle::Particle(const Vector3& position, const Vector3& velocity, float scale, float gravityScale, float lifetime) 
	: m_position(position)
	, m_velocity(velocity)
	, m_gravityScale(gravityScale)
	, m_lifetime(lifetime)
	, m_scale(scale)
	, m_elapsedTime(0.f)
	, m_alive(true)
{
}

Particle::~Particle() {
}

void Particle::update(float dt) {
	if (m_alive) {
		m_velocity += Vector3(0.f, -9.82f, 0.f) * m_gravityScale * dt;
		m_position += m_velocity * dt;
		m_elapsedTime += dt;
		//m_elapsedTime += 0.6f;
		if (m_elapsedTime > m_lifetime) {
			m_alive = false;
			m_elapsedTime = m_lifetime;
		}
	}
}

const DirectX::SimpleMath::Vector3& Particle::getPosition() const {
	return m_position;
}

float Particle::getLifePercentage() const {
	return m_elapsedTime / m_lifetime;
}

bool Particle::isDead() const {
	return !m_alive;
}
