#include "pch.h"
#include "Sail.h"
#include "Projectile.h"

Projectile::Projectile() {
	m_projectile = ECS::Instance()->createEntity("gun_projectile");

	//m_model = ModelFactory::CubeModel::Create(glm::vec3(0.5f), shader);
	//m_model->getMesh(0)->getMaterial()->setColor(glm::vec4(0.2f, 0.8f, 0.4f, 1.0f));

	//m_projectile->addComponent<ModelComponent>(model);
}