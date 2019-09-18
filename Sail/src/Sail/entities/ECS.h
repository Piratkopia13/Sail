#pragma once
#include "Entity.h"
#include "systems/BaseComponentSystem.h"
#include <unordered_map>
#include <typeindex>
#include <memory>

class ECS final {
public:

	/*
		Retrieves a pointer to an instance of this class
		Needs to be called to access ECS unless a direct pointer is stored
	*/
	static ECS* Instance() {
		static ECS instance;
		return &instance;
	}

	/*
		Creates and adds an entity
	*/
	Entity::SPtr createEntity(const std::string& name = "");

	void destroyEntity(const Entity::SPtr entityToRemove);

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
		Returns a system
	*/
	template<typename SystemType>
	SystemType* getSystem();

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
	void addEntityToSystems(Entity::SPtr entity);
	void removeEntityFromSystems(Entity::SPtr entity);

private:
	typedef std::unordered_map<std::type_index, std::unique_ptr<BaseComponentSystem>> SystemMap;

	ECS();
	~ECS();

	std::vector<Entity::SPtr> m_entities;
	SystemMap m_systems;
};

template<typename T>
inline void ECS::addSystem(BaseComponentSystem* system) {
	SystemMap::iterator it = m_systems.find(typeid(T));
	if (it == m_systems.end()) {
		m_systems[typeid(T)] = std::unique_ptr<T>(system);
	}
}

template<typename T>
inline void ECS::createSystem() {
	SystemMap::iterator it = m_systems.find(typeid(T));
	if (it == m_systems.end()) {
		m_systems[typeid(T)] = std::make_unique<T>();
	}
}

template<typename SystemType>
inline SystemType* ECS::getSystem() {
	SystemMap::iterator it = m_systems.find(typeid(SystemType));
	if (it != m_systems.end()) {
		return static_cast<SystemType*>(it->second.get());
	}

	return nullptr;
}
