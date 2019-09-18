#pragma once
#include <vector>

class Entity;

/*
	Created systems must inherit from this class.
	In the sub-system's default constructor (or if it has an initialize function),
	add each component type required for that system to m_requiredComponentTypes.
	Any entity in m_entities WILL have all of the required components, so no checks are needed.
	However, there can still be optional components if the system wants it, which should be checked before used.

	Example: See PhysicSystem.h and PhysicSystem.cpp
*/
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