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

void CandleComponent::putDown() {
	m_carried = false;
}

void CandleComponent::pickUp() {
	m_carried = true;

}

bool CandleComponent::isCarried() const {
	return m_carried;
}