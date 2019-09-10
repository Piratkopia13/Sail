#pragma once

#include "../Entity.h"

class SimplePhysicsSystem {
public:
	SimplePhysicsSystem();
	~SimplePhysicsSystem();

	void registerEntity(Entity::SPtr entity);

	void execute(float dt);
	
private:
	std::vector<Entity::SPtr> m_entities;

};