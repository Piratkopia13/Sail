#pragma once

#include "DirectionalLight.h"
#include "PointLight.h"
#include <vector>


class LightSetup {
public:

	static const UINT MAX_POINTLIGHTS_FORWARD_RENDERING = 12;  // Max number of lights as set in shader

	struct PointLightStruct {
		glm::vec3 color = glm::vec3(0.f);
		float padding;
		glm::vec3 position = glm::vec3(0.f);
		float attConstant = 0.f;
		float attLinear;
		float attQuadratic;
		float padding1, padding2;
	};

	struct SpotlightStruct {
		// This part must match point light input
		// (all lights are casted to PointLightInput during shading)
		glm::vec3 color = glm::vec3(0.f);
		float padding;
		glm::vec3 position = glm::vec3(0.f);
		float attConstant = 0.f;
		float attLinear;
		float attQuadratic;
		float padding1, padding2;
		// This part can be unique for each light type
		glm::vec3 direction = glm::vec3(1.f, 0.0f, 0.0f);
		float angle;
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

	struct SpotlightBuffer {
		SpotlightBuffer() {};
		SpotlightStruct sLights[MAX_POINTLIGHTS_FORWARD_RENDERING];
	};


public:
	LightSetup();
	~LightSetup();

	void addPointLight(const PointLight& pl);
	void addSpotLight(const SpotLight& pl);
	void setDirectionalLight(const DirectionalLight& dl);

	const DirectionalLight& getDL() const;
	std::vector<PointLight>& getPLs();
	std::vector<SpotLight>& getSLs();

	const DirLightBuffer& getDirLightData() const;
	const PointLightsBuffer& getPointLightsData() const;
	const SpotlightBuffer& getSpotLightsData() const;
	void clearPointLights();
	void removePointLight();
	void removePLByIndex(int index);
	void updateBufferData();
private:

	DirectionalLight m_dl;
	std::vector<PointLight> m_pls;
	std::vector<SpotLight> m_sls;

	int m_numPls;
	int m_numSls;

	DirLightBuffer m_dlData;
	PointLightsBuffer m_plData;
	SpotlightBuffer m_slData;

};