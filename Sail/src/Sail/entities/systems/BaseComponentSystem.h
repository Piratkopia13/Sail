#pragma once
class Entity;

class BaseComponentSystem
{
public:
	BaseComponentSystem() {}
	virtual ~BaseComponentSystem() {}

	void addEntity(Entity* entity);

	virtual void update(float dt) = 0;

	const std::vector<int>& getRequiredComponentTypes() const;

protected:
	std::vector<Entity*> m_entities;
	std::vector<int> m_requiredComponentTypes;
};