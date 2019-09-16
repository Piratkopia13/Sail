#pragma once
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <vector>
#include "Entity.h"
#include "systems/BaseComponentSystem.h"
#include "storages/BaseComponentStorage.h"

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
		Updates every system
	*/
	//void update(float dt);


	/*
		Creates and adds an entity
	*/
	Entity::SPtr createEntity(const std::string& name = "");

	/*
		Destroys an entity and removes it from the systems it was stored in
	*/
	void destroyEntity(const Entity::SPtr entityToRemove);

	/*
		Adds an already existing system of a chosen type
		Will not add if one of the same type already has been added or created
	*/
	template<typename T>
	void addSystem(T* system);

	/*
		Creates and adds a system of a chosen type
		Will not create if one of the same type already has been added or created
	*/
	template<typename T>
	T* createSystem();

	/*
		Retrieves a system of the chosen type if it has been created or added
		Otherwise a null pointer is returned
	*/
	template<typename T>
	T* getSystem();

	/*
		Adds an already existing storage of a chosen type
		Will not add if one of the same type already has been added or created
	*/
	template<typename T>
	void addStorage(T* storage);

	/*
		Creates and adds a storage of a chosen type
		Will not create if one of the same type already has been added or created
	*/
	template<typename T>
	T* createStorage();

	/*
	Retrieves a storage of the chosen type if it has been created or added
	Otherwise a null pointer is returned
	*/
	template<typename T>
	T* getStorage();


	/*
		Returns the number of unique component types
	*/
	unsigned nrOfComponentTypes() const;


	/*
		Should NOT be called by the game developer
		This are called internally by Entity
	*/
	void addEntityToSystems(Entity* entity);

	/*
		Should NOT be called by the game developer
		This are called internally by Entity
	*/
	void removeEntityFromSystems(Entity* entity);

private:
	typedef std::unordered_map<std::type_index, std::unique_ptr<BaseComponentSystem>> SystemMap;
	typedef std::unordered_map<ComponentTypeID, std::unique_ptr<BaseComponentStorage>> StorageMap;

	ECS();
	~ECS();

	std::vector<Entity::SPtr> m_entities;
	SystemMap m_systems;
	StorageMap m_storages;
};

template<typename T>
inline void ECS::addSystem(T* system) {
	SystemMap::iterator it = m_systems.find(typeid(T));
	if (it == m_systems.end()) {
		m_systems[typeid(T)] = std::unique_ptr<T>(system);
	}
}

template<typename T>
inline T* ECS::createSystem() {
	SystemMap::iterator it = m_systems.find(typeid(T));
	if (it == m_systems.end()) {
		m_systems[typeid(T)] = std::make_unique<T>();
	}
	return static_cast<T*>(m_systems.at(typeid(T)));
}

template<typename T>
inline T* ECS::getSystem() {
	SystemMap::iterator it = m_systems.find(typeid(T));
	if (it == m_systems.end()) {
		return nullptr;
	}
	return static_cast<T*>(it->second.get());
}

template<typename T>
inline void ECS::addStorage(T* storage) {
	StorageMap::iterator it = m_storages.find(T::ID);
	if (it == m_storages.end()) {
		m_storages[T::ID] = std::unique_ptr<T>(storage);
	}
}

template<typename T>
inline T* ECS::createStorage() {
	StorageMap::iterator it = m_storages.find(T::ID);
	if (it == m_storages.end()) {
		m_storages[T::ID] = std::make_unique<T>();
	}
	return static_cast<T*>(it->second);
}

template<typename T>
inline T* ECS::getStorage() {
	SystemMap::iterator it = m_systems.find(T::ID);
	if (it == m_systems.end()) {
		return nullptr;
	}
	return static_cast<T*>(it->second.get());
}
