#pragma once
#include "Entity.h"
#include "systems/BaseComponentSystem.h"
#include <unordered_map>
#include <typeindex>
#include <memory>

class ECS final {
public:
	static ECS* Instance() {
		static ECS instance;
		return &instance;
	}

	Entity::SPtr createEntity(const std::string& name = "");

	template<typename T>
	void addSystem(BaseComponentSystem* system);
	template<typename T>
	void createSystem();

	void update(float dt);


	/*
		Should NOT be called by the game developer
		This is called internally by Entity
	*/
	void addEntityToSystems(Entity* entity);

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