#include "pch.h"
#include "LightSetup.h"



LightSetup::LightSetup()
	: m_numPls(0)
{ }
LightSetup::~LightSetup() {}

void LightSetup::addPointLight(const PointLight& pl) {
	UINT ind = Scene::GetUpdateIndex();
	m_pls[ind].push_back(pl);
	if (pl.getIndex() < 0) {
		m_pls[ind].back().setIndex(m_pls[ind].size() + 12);
	}
	//std::cout << "added ligth " << m_pls.size() << std::endl;
	//updateBufferData();
}
void LightSetup::setDirectionalLight(const DirectionalLight& dl) {
	m_dl = dl;
	updateBufferData();
}

const DirectionalLight& LightSetup::getDL() const {
	return m_dl;
}
const std::vector<PointLight>& LightSetup::getPLs() const {
	return m_pls[Scene::GetRenderIndex()];
}

const LightSetup::DirLightBuffer& LightSetup::getDirLightData() const {
	return m_dlData;
}

const LightSetup::PointLightsBuffer& LightSetup::getPointLightsData() const {
	return m_plData;
}

void LightSetup::clearPointLights() {
	m_pls[Scene::GetUpdateIndex()].clear();
	//updateBufferData();
}

void LightSetup::removePointLight() {
	UINT ind = Scene::GetUpdateIndex();
	if (m_pls[ind].size() > 0) {
		m_pls[ind].erase(m_pls[ind].begin());
		std::cout << "removed light " << m_pls[ind].size() << std::endl;
		//updateBufferData();
	}
}

void LightSetup::removePLByIndex(int index) {
	UINT ind = Scene::GetUpdateIndex();
	for (unsigned int i = 0; i < m_pls[ind].size(); i++) {
		if (m_pls[ind][i].getIndex() == index) {
			m_pls[ind].erase(m_pls[ind].begin()+i);
			std::cout << "removed lightIndex " << i<<std::endl;
		}
	}
	//updateBufferData();
}

void LightSetup::updateBufferData() {
	UINT ind = Scene::GetRenderIndex();
	m_dlData.color = m_dl.getColor();
	m_dlData.direction = m_dl.getDirection();
	// Copy the x first lights into the buffer
		for (unsigned int i = 0; i < MAX_POINTLIGHTS_FORWARD_RENDERING; i++) {
			if (i < m_pls[ind].size()) {// break;
				m_plData.pLights[i].attConstant = m_pls[ind][i].getAttenuation().constant;
				/*m_plData.pLights[i].attLinear = m_pls[i].getAttenuation().linear;
				m_plData.pLights[i].attQuadratic = m_pls[i].getAttenuation().quadratic;*/
				m_plData.pLights[i].color = m_pls[ind][i].getColor();
				m_plData.pLights[i].position = m_pls[ind][i].getPosition();
			} else {
				m_plData.pLights[i].attConstant = 0;
				m_plData.pLights[i].color = glm::vec3(0.f, 0.f, 0.f);
				m_plData.pLights[i].position = glm::vec3(0.f, 0.f, 0.f);
			}
		}
}