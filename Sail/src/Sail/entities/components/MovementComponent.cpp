#include "pch.h"
#include "MovementComponent.h"



MovementComponent::MovementComponent(const float& initialSpeed, const glm::vec3& initialDirection) {
	m_speed = initialSpeed;
	m_direction = initialDirection;
}

MovementComponent::~MovementComponent() {}

void MovementComponent::setSpeed( const float& speed ) {
	m_speed = speed;
}

void MovementComponent::addSpeed(const float& speed) {
	m_speed += speed;
}

void MovementComponent::setDirection( const glm::vec3& direction ) {
	m_direction = direction;
}

const float& MovementComponent::getSpeed() const {
	return m_speed;
}

const glm::vec3& MovementComponent::getDirection() const {
	return m_direction;
}
