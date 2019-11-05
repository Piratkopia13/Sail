#include "pch.h"
#include "LightSetup.h"
#include "Sail/utils/Utils.h"


LightSetup::LightSetup() : m_numPls(0) { 
	updateBufferData();
}

LightSetup::~LightSetup() {}

void LightSetup::addPointLight(const PointLight& pl) {
	m_pls.push_back(pl);
	if (pl.getIndex() < 0) {
		Logger::Warning("Pointlight Index Less than 0. Index: " + std::to_string(pl.getIndex()));
		m_pls.back().setIndex(m_pls.size() + 12);
	}
}
void LightSetup::addSpotLight(const SpotLight& pl) {
	m_sls.push_back(pl);
	if (pl.getIndex() < 0) {
		Logger::Warning("Spotlight Index Less than 0. Index: " + std::to_string(pl.getIndex()));
		m_sls.back().setIndex(m_sls.size() + 12);
	}
}

void LightSetup::setDirectionalLight(const DirectionalLight& dl) {
	m_dl = dl;
	updateBufferData();
}

const DirectionalLight& LightSetup::getDL() const {
	return m_dl;
}
std::vector<PointLight>& LightSetup::getPLs() {
	return m_pls;
}

std::vector<SpotLight>& LightSetup::getSLs() {
	return m_sls;
}

const LightSetup::DirLightBuffer& LightSetup::getDirLightData() const {
	return m_dlData;
}

const LightSetup::PointLightsBuffer& LightSetup::getPointLightsData() const {
	return m_plData;
}

const LightSetup::SpotlightBuffer& LightSetup::getSpotLightsData() const {
	return m_slData;
}

void LightSetup::clearPointLights() {
	m_pls.clear();
	//updateBufferData();
}

void LightSetup::removePointLight() {
	if (m_pls.size() > 0) {
		m_pls.erase(m_pls.begin());
		std::cout << "removed light " << m_pls.size() << std::endl;
		//updateBufferData();
	}
}

// Not used at the moment
void LightSetup::removePLByIndex(int index) {
	for (unsigned int i = 0; i < m_pls.size(); i++) {
		if (m_pls[i].getIndex() == index) {
			m_pls.erase(m_pls.begin()+i);
			std::cout << "removed lightIndex " << i<<std::endl;
		}
	}
	//updateBufferData();
}

void LightSetup::updateBufferData() {
	m_dlData.color = m_dl.getColor();
	m_dlData.direction = m_dl.getDirection();
	// Copy the x first lights into the buffer
	for (unsigned int i = 0; i < MAX_POINTLIGHTS_FORWARD_RENDERING; i++) {
		//Pointlights
		if (i < m_pls.size()) {// break;
			m_plData.pLights[i].attConstant = m_pls[i].getAttenuation().constant;
			m_plData.pLights[i].attLinear = m_pls[i].getAttenuation().linear;
			m_plData.pLights[i].attQuadratic = m_pls[i].getAttenuation().quadratic;
			m_plData.pLights[i].color = m_pls[i].getColor();
			m_plData.pLights[i].position = m_pls[i].getPosition();
		} else {
			m_plData.pLights[i].attConstant = 0.312f;
			m_plData.pLights[i].attLinear = 0.0f;
			m_plData.pLights[i].attQuadratic = 0.0009f;
			m_plData.pLights[i].color = glm::vec3(0.f, 0.f, 0.f);
			m_plData.pLights[i].position = glm::vec3(0.f, 0.f, 0.f);
		}

		//Spotlights
		if (i < m_sls.size()) {// break;
			m_slData.sLights[i].attConstant		= m_sls[i].getAttenuation().constant;
			m_slData.sLights[i].attLinear		= m_sls[i].getAttenuation().linear;
			m_slData.sLights[i].attQuadratic	= m_sls[i].getAttenuation().quadratic;
			m_slData.sLights[i].color			= m_sls[i].getColor();
			m_slData.sLights[i].position		= m_sls[i].getPosition();
			m_slData.sLights[i].direction		= m_sls[i].getDirection();
			m_slData.sLights[i].angle			= m_sls[i].getAngle();
		} else {
			m_slData.sLights[i].attConstant = 0.312f;
			m_slData.sLights[i].attLinear = 0.0f;
			m_slData.sLights[i].attQuadratic = 0.0009f;
			m_slData.sLights[i].color = glm::vec3(0.f, 0.f, 0.f);
			m_slData.sLights[i].position = glm::vec3(0.f, 0.f, 0.f);
			m_slData.sLights[i].direction = glm::vec3(0.f, 0.f, 0.f);
			m_slData.sLights[i].angle = 0;
		}
	}
}