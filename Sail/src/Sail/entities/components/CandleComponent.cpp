#include "pch.h"
#include "CandleComponent.h"

CandleComponent::CandleComponent() {

}

CandleComponent::~CandleComponent() {

}

void CandleComponent::hitWithWater(float damage) {
	m_damageTakenLastHit = damage;
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

bool* CandleComponent::getPtrToIsAlive() {
	return &m_isAlive;
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

void CandleComponent::activate() {
	if ( !m_activate && m_isAlive && !m_isLit ) {
		m_activate = true;
	}
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

bool CandleComponent::getIsLit() const {
	return m_isLit;
}

void CandleComponent::setIsLit(const bool isLit) {
	m_isLit = isLit;
}

int CandleComponent::getNumRespawns() const {
	return m_respawns;
}

void CandleComponent::incrementRespawns() {
	m_respawns++;
}

void CandleComponent::setOwner(int playerEntityID) {
	m_playerEntityID = playerEntityID;
}

int CandleComponent::getOwner() const {
	return m_playerEntityID;
}

int CandleComponent::getDamageTakenLastHit() const {
	return m_damageTakenLastHit;
}

bool CandleComponent::isCarried() const {
	return m_carried;
}

void CandleComponent::setWasCarriedLastUpdate(const bool wasCarried) {
	m_wasCarriedLastUpdate = wasCarried;
}

bool CandleComponent::getWasCarriedLastUpdate() const {
	return m_wasCarriedLastUpdate;
}

void CandleComponent::toggleCarried() {
	m_carried = !m_carried;
}