#pragma once

#include "DirectionalLight.h"
#include "PointLight.h"
#include <vector>

class LightSetup {
public:

	static const UINT MAX_POINTLIGHTS_FORWARD_RENDERING = 8;  // Max number of lights as set in shader

	struct PointLightStruct {
		glm::vec3 color = glm::vec3(0.f);
		float padding;
		glm::vec3 position = glm::vec3(0.f);
		float attConstant = 0.f;
		float attLinear;
		float attQuadratic;
		float padding1, padding2;
	};
	struct DirLightBuffer {
		DirLightBuffer() { };
		glm::vec3 color = glm::vec3(0.f);
		float padding1;
		glm::vec3 direction = glm::vec3(0.f);
		float padding2;
	};
	struct PointLightsBuffer {
		PointLightsBuffer() { };
		PointLightStruct pLights[MAX_POINTLIGHTS_FORWARD_RENDERING];
	};


public:
	LightSetup();
	~LightSetup();

	void addPointLight(const PointLight& pl);
	void setDirectionalLight(const DirectionalLight& dl);

	const DirectionalLight& getDL() const;
	std::vector<PointLight>& getPLs();

	const DirLightBuffer& getDirLightData() const;
	const PointLightsBuffer& getPointLightsData() const;
	void clearPointLights();
	void removePointLight();
	void removePLByIndex(int index);
	void updateBufferData();
private:

	DirectionalLight m_dl;
	std::vector<PointLight> m_pls;
	int m_numPls;

	DirLightBuffer m_dlData;
	PointLightsBuffer m_plData;

};