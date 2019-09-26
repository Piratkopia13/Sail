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
