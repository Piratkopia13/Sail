#pragma once

#include <unordered_map>
#include <memory>
#include "components/Component.h"

//#define MOVE(x) std::move(x)

class ECS;

class Entity {
public:
	typedef std::shared_ptr<Entity> SPtr;
public:
	virtual ~Entity();

	template<typename T, typename... Targs>
	T* addComponent(Targs... args);
	template<typename T>
	T* getComponent();
	
	template<typename T>
	bool hasComponent() const;
	bool hasComponent(int id) const;
	
	void setName(const std::string& name);
	const std::string& getName() const;
	int getID() const;
	Entity(const std::string& name = "");

private:
	// Only ECS should be able to create entities
	friend class ECS;
	static SPtr Create(ECS* ecs, const std::string& name = "");

	void addToSystems();

	std::unordered_map<int, Component::Ptr> m_components;
	std::string m_name;
	int m_id;
	ECS* m_ecs;
};

template<typename T, typename... Targs>
T* Entity::addComponent(Targs... args) {
	auto res = m_components.insert({ T::getStaticID(), std::make_unique<T>(args...) });
	if (!res.second) {
		Logger::Warning("Tried to add a duplicate component to an entity");
	}

	// Place this entity within the correct systems
	addToSystems();

	// Return pointer to the inserted component
	return static_cast<T*>(res.first->second.get());
}

template<typename T>
T* Entity::getComponent() {
	// If the following line causes compile errors, then a class 
	// deriving from component is missing public SAIL_COMPONENT macro
	auto it = m_components.find(T::getStaticID());
	if (it != m_components.end())
		return static_cast<T*>(it->second.get());

	return nullptr;
}

template<typename T>
inline bool Entity::hasComponent() const
{
	return hasComponent(T::getStaticID());
}
