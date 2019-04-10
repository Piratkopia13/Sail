#pragma once

#include "DirectionalLight.h"
#include "PointLight.h"
#include <vector>

class LightSetup {
public:

	static const UINT MAX_POINTLIGHTS_FORWARD_RENDERING = 8;  // Max number of lights as set in shader

	struct PointLightStruct {
		glm::vec3 color;
		float padding;
		glm::vec3 position;
		float attConstant;
		/*float attLinear;
		float attQuadratic;
		float padding1, padding2;*/
	};
	struct DirLightBuffer {
		DirLightBuffer() { };
		glm::vec3 color;
		float padding1;
		glm::vec3 direction;
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
	const std::vector<PointLight>& getPLs() const;

	const DirLightBuffer& getDirLightData() const;
	const PointLightsBuffer& getPointLightsData() const;

private:
	void updateBufferData();
private:

	DirectionalLight m_dl;
	std::vector<PointLight> m_pls;
	int m_numPls;

	DirLightBuffer m_dlData;
	PointLightsBuffer m_plData;

};