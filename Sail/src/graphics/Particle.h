#pragma once

#include <d3d11.h>
#include <SimpleMath.h>

class Particle {
public:
	Particle(const DirectX::SimpleMath::Vector3& position, const DirectX::SimpleMath::Vector3& velocity, float scale = 1.0f, float gravityScale = 0.f, float lifetime = 10.f);
	~Particle();

	void update(float dt);
	const DirectX::SimpleMath::Vector3& getPosition() const;
	float getLifePercentage() const;
	bool isDead() const;

private:
	DirectX::SimpleMath::Vector3 m_position;
	DirectX::SimpleMath::Vector3 m_velocity;
	float m_gravityScale;
	float m_lifetime;
	float m_scale;
	float m_elapsedTime;
	bool m_alive;

};