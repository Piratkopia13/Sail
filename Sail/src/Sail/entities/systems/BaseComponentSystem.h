#pragma once
#include <vector>

class Entity;

class BaseComponentSystem
{
public:
	BaseComponentSystem() {}
	virtual ~BaseComponentSystem() {}

	virtual void update(float dt) = 0;

	/*
		Adds an entity to the system
	*/
	void addEntity(Entity* entity);

	/*
		Removes an entity from the system
	*/
	void removeEntity(Entity* entity);

	/*
		Returns the indices of all the component types required to be within this system
	*/
	const std::vector<int>& getRequiredComponentTypes() const;

protected:
	std::vector<Entity*> m_entities;
	std::vector<int> m_requiredComponentTypes;
};