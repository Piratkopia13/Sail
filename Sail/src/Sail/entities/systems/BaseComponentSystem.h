#pragma once

#include "..//Entity.h"

class BaseComponentSystem
{
public:
	BaseComponentSystem() {}
	virtual ~BaseComponentSystem() {}

	virtual void update(float dt) = 0;

	/*
		Adds an entity to the system
	*/
	virtual void addEntity(Entity::SPtr entity);

	/*
		Removes an entity from the system
	*/
	virtual void removeEntity(Entity::SPtr entity);

	/*
		Returns the indices of all the component types required to be within this system
	*/
	const std::vector<int>& getRequiredComponentTypes() const;

protected:
	std::vector<Entity::SPtr> entities;
	std::vector<int> requiredComponentTypes;
};