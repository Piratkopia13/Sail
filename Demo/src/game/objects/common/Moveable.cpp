#include "Moveable.h"

using namespace DirectX::SimpleMath;

Moveable::Moveable() {
	m_velocity = Vector3::Zero;
	m_gravity = Vector3(0.f, -9.82f, 0.f);
	m_acceleration = Vector3::Zero;
	m_gravScale = 1;
	m_grounded = true;
}

Moveable::~Moveable() {
}

void Moveable::move(const float dt, bool includeBoundingBoxRotation) {
	Moveable::move(m_velocity * dt, includeBoundingBoxRotation);
}
void Moveable::updateVelocity(const float dt) {
	if (!m_grounded)
		m_velocity += (m_gravity*m_gravScale + m_acceleration) * dt;
	else
		m_velocity += m_acceleration * dt;
}

void Moveable::move(const Vector3& toMove, bool includeBoundingBoxRotation) {
	this->getTransform().translate(toMove);
	this->updateBoundingBox(includeBoundingBoxRotation);
}

void Moveable::setVelocity(const Vector3 &newVelocity) {
	m_velocity = newVelocity;
}

void Moveable::addVelocity(const DirectX::SimpleMath::Vector3 & addedVelocity) {
	m_velocity += addedVelocity;
}

void Moveable::setGravScale(float scale) {
	m_gravScale = scale;
}

const Vector3& Moveable::getVelocity() {
	return m_velocity;
}
void Moveable::setAcceleration(const Vector3 &newAcceleration) {
	m_acceleration = newAcceleration;
}

void Moveable::addAcceleration(const Vector3& accel) {
	this->m_acceleration += accel;
}

void Moveable::setGrounded(bool grounded) {
	m_grounded = grounded;
}

bool Moveable::grounded() {
	return m_grounded;
}