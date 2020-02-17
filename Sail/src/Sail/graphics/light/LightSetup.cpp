#include "pch.h"
#include "LightSetup.h"
#include "Sail/entities/components/PointLightComponent.h"
#include "Sail/entities/components/DirectionalLightComponent.h"

LightSetup::LightSetup() {
	m_plData.resize(MAX_POINTLIGHTS_FORWARD_RENDERING);
	m_numPls = 0;
}
LightSetup::~LightSetup() {}

void LightSetup::addPointLight(PointLightComponent* plComp) {
	if (m_numPls >= m_plData.size()) return;

	auto& pl = m_plData[m_numPls++];
	pl.attRadius = plComp->getAttenuationRadius();
	pl.color = plComp->getColor();
	pl.position = plComp->getPosition();
	pl.intensity = plComp->getIntensity();
}

void LightSetup::setDirectionalLight(DirectionalLightComponent* dl) {
	m_dlData.color = dl->getColor();
	m_dlData.direction = dl->getDirection();
	m_dlData.intensity = dl->getIntensity();
}

std::tuple<void*, unsigned int> LightSetup::getDirLightData() const {
	return { (void*)&m_dlData, sizeof(DirLightBuffer) };
}

std::tuple<void*, unsigned int> LightSetup::getPointLightsData() const {
	return { (void*)m_plData.data(), sizeof(PointLightStruct) * MAX_POINTLIGHTS_FORWARD_RENDERING };
}

unsigned int LightSetup::getNumPLs() const {
	return m_numPls;
}
