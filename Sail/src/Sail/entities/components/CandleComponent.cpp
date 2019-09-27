#include "pch.h"
#include "CandleComponent.h"

CandleComponent::CandleComponent() {

}

CandleComponent::~CandleComponent() {

}

void CandleComponent::hitWithWater() {
	m_wasHitByWater = true;
}

void CandleComponent::resetHitByWater() {
	m_wasHitByWater = false;
}

bool CandleComponent::wasHitByWater() const {
	return m_wasHitByWater;
}

bool CandleComponent::getIsAlive() const {
	return m_isAlive;
}

void CandleComponent::setIsAlive(bool alive) {
	if ( !m_isAlive && alive ) {
		m_activate = true;
		m_isAlive = alive;
	}
	m_isAlive = alive;
}

bool CandleComponent::getDoActivate() const {
	return m_activate;
}

void CandleComponent::resetDoActivate() {
	m_activate = false;
}

void CandleComponent::addToDownTime(float time) {
	m_downTime += time;
}

void CandleComponent::resetDownTime() {
	m_downTime = 0.f;
}

float CandleComponent::getDownTime() const {
	return m_downTime;
}
