#include "pch.h"
#include "LightSetup.h"

LightSetup::LightSetup()
	: m_numPls(0)
{ }
LightSetup::~LightSetup() {}

void LightSetup::addPointLight(const PointLight& pl) {
	m_pls.push_back(pl);
	//m_pls.back().setIndex(m_pls.size());
	std::cout << "added ligth " << m_pls.size() << std::endl;
	updateBufferData();
}
void LightSetup::setDirectionalLight(const DirectionalLight& dl) {
	m_dl = dl;
	updateBufferData();
}

const DirectionalLight& LightSetup::getDL() const {
	return m_dl;
}
const std::vector<PointLight>& LightSetup::getPLs() const {
	return m_pls;
}

const LightSetup::DirLightBuffer& LightSetup::getDirLightData() const {
	return m_dlData;
}

const LightSetup::PointLightsBuffer& LightSetup::getPointLightsData() const {
	return m_plData;
}

void LightSetup::removePointLight() {
	if (m_pls.size() > 0) {
		m_pls.erase(m_pls.begin());
		std::cout << "removed light " << m_pls.size() << std::endl;
		updateBufferData();
	}
}

void LightSetup::updateBufferData() {
	m_dlData.color = m_dl.getColor();
	m_dlData.direction = m_dl.getDirection();
	// Copy the x first lights into the buffer
	if (m_pls.size() == 0) {
		for (unsigned int i = 0; i < MAX_POINTLIGHTS_FORWARD_RENDERING; i++) {
			m_plData.pLights[i].attConstant = 0;
			m_plData.pLights[i].color = glm::vec3(0.f, 0.f, 0.f);
			m_plData.pLights[i].position = glm::vec3(0.f, 0.f, 0.f);
		}
	}
	else {
		for (unsigned int i = 0; i < MAX_POINTLIGHTS_FORWARD_RENDERING; i++) {
			if (i >= m_pls.size()) break;
			m_plData.pLights[i].attConstant = m_pls[i].getAttenuation().constant;
			/*m_plData.pLights[i].attLinear = m_pls[i].getAttenuation().linear;
			m_plData.pLights[i].attQuadratic = m_pls[i].getAttenuation().quadratic;*/
			m_plData.pLights[i].color = m_pls[i].getColor();
			m_plData.pLights[i].position = m_pls[i].getPosition();
		}
	}
}