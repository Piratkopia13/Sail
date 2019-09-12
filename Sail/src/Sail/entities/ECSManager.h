#pragma once
#include "Entity.h"
#include "systems/BaseComponentSystem.h"
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <vector>

class ECSManager final {
public:

	/*
		Retrieves a pointer to an instance of this class
		Needs to be called to access ECS unless a direct pointer is stored
	*/
	static ECSManager* Instance() {
		static ECSManager instance;
		return &instance;
	}

	/*
		Creates and adds an entity
	*/
	Entity::SPtr createEntity(const std::string& name = "");

	/*
		Adds an already existing system of a chosen type
		Will not add if one of the same type already has been added or created
	*/
	template<typename T>
	void addSystem(BaseComponentSystem* system);

	/*
		Creates and adds a system of a chosen type
		Will not create if one of the same type already has been added or created
	*/
	template<typename T>
	void createSystem();

	/*
		Updates every system
	*/
	void update(float dt);

	/*
		Returns the number of unique component types
	*/
	unsigned nrOfComponentTypes() const;

	/*
		Should NOT be called by the game developer
		These are called internally by Entity
	*/
	void addEntityToSystems(Entity* entity);
	void removeEntityFromSystems(Entity* entity);

private:
	typedef std::unordered_map<std::type_index, std::unique_ptr<BaseComponentSystem>> SystemMap;

	ECSManager();
	~ECSManager();

	std::vector<Entity::SPtr> m_entities;
	SystemMap m_systems;
};

template<typename T>
inline void ECSManager::addSystem(BaseComponentSystem* system) {
	SystemMap::iterator it = m_systems.find(typeid(T));
	if (it == m_systems.end()) {
		m_systems[typeid(T)] = std::unique_ptr<T>(system);
	}
}

template<typename T>
inline void ECSManager::createSystem() {
	SystemMap::iterator it = m_systems.find(typeid(T));
	if (it == m_systems.end()) {
		m_systems[typeid(T)] = std::make_unique<T>();
	}
}