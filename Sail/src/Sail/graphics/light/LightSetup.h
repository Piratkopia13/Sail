#pragma once

#include <vector>

class DirectionalLightComponent;
class PointLightComponent;

class LightSetup {
public:
	static const UINT MAX_POINTLIGHTS_FORWARD_RENDERING = 128;  // Max number of lights as set in shader

	struct PointLightStruct {
		PointLightStruct() { }
		glm::vec3 color = glm::vec3(0.f);
		float attRadius = 10.f;
		glm::vec3 position = glm::vec3(0.f);
		float intensity = 10.f;
	};
	struct DirLightBuffer {
		DirLightBuffer() { }
		glm::vec3 color = glm::vec3(0.f);
		float intensity = 1.f;
		glm::vec3 direction = glm::vec3(0.f);
		float padding2;
	};

public:
	LightSetup();
	~LightSetup();

	void addPointLight(PointLightComponent* plComp);
	void setDirectionalLight(DirectionalLightComponent* dl);

	std::tuple<void*, unsigned int> getDirLightData() const;
	std::tuple<void*, unsigned int> getPointLightsData() const;
	unsigned int getNumPLs() const;

private:
	DirLightBuffer m_dlData;
	std::vector<PointLightStruct> m_plData;
	unsigned int m_numPls;

};