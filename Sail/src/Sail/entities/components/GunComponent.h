#pragma once
#include "Component.h"

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

	float projectileSpawnTimer = 0.f;
	float projectileSpeed = 20.f;
	bool firing = false;

private:
	Model* m_projectileModel;
	Model* m_wireframeModel;


	float m_projectileSpawnLimit = 0.05f;
};
