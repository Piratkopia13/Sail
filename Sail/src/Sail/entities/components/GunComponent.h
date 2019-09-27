#pragma once
#include "Component.h"
#include <glm/glm.hpp>

class Model;
//class Transform;

class GunComponent : public Component<GunComponent> {
public:
	GunComponent(Model* projectileModel) : 
	m_projectileModel(projectileModel)
	,m_wireframeModel(projectileModel)
	{ };
	~GunComponent() { };

	Model* getProjectileModel() const {
		return m_projectileModel;
	}

	Model* getWireframeModel() const {
		return m_wireframeModel;
	}

	float getSpawnLimit() const {
		return m_projectileSpawnLimit;
	}

	void setFiring(glm::vec3 pos, glm::vec3 dir);

	glm::vec3 position;
	glm::vec3 direction;

	float projectileSpawnTimer = 0.f;
	float projectileSpeed = 70.f;
	bool firing = false;

private:
	Model* m_projectileModel;
	Model* m_wireframeModel;


	float m_projectileSpawnLimit = 0.1f;
};
