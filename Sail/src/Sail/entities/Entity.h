#pragma once

#include <unordered_map>
#include <memory>
#include "components/Component.h"

//#define MOVE(x) std::move(x)

class ECSManager;


class Entity {
public:
	typedef std::shared_ptr<Entity> SPtr;
public:
	virtual ~Entity();

	template<typename ComponentType, typename... Targs>
	ComponentType* addComponent(Targs... args);
	template<typename ComponentType>
	void removeComponent();
	template<typename ComponentType>
	ComponentType* getComponent();
	template<typename ComponentType>
	bool hasComponent() const;

	bool hasComponent(int id) const;
	
	void setName(const std::string& name);
	const std::string& getName() const;
	int getID() const;
	Entity(const std::string& name = "");

private:
	// Only ECS should be able to create entities
	friend class ECSManager;
	static SPtr Create(ECSManager* ecs, const std::string& name = "");

	void addToSystems();
	void removeFromSystems();

	std::unordered_map<int, BaseComponent::Ptr> m_components;
	std::string m_name;
	int m_id;
	ECSManager* m_ecs;
};

template<typename ComponentType, typename... Targs>
ComponentType* Entity::addComponent(Targs... args) {
	auto res = m_components.insert({ ComponentType::ID, std::make_unique<ComponentType>(args...) });
	if (!res.second) {
		Logger::Warning("Tried to add a duplicate component to an entity");
	}

	// Place this entity within the correct systems
	addToSystems();

	// Return pointer to the inserted component
	return static_cast<ComponentType*>(res.first->second.get());
}

template<typename ComponentType>
inline void Entity::removeComponent() {
	auto it = m_components.find(ComponentType::ID);
	if (it != m_components.end()) {
		// Simply erasing the result of find() appears to be undefined behavior if the iterator points to end()
		m_components.erase(it);

		// Remove this entity from systems which required the removed component
		removeFromSystems();
	}
}

template<typename ComponentType>
ComponentType* Entity::getComponent() {
	// If the following line causes compile errors, then a class 
	// deriving from component is missing public SAIL_COMPONENT macro
	auto it = m_components.find(ComponentType::ID);
	if (it != m_components.end()) {
		return static_cast<ComponentType*>(it->second.get());
	}

	return nullptr;
}

template<typename ComponentType>
inline bool Entity::hasComponent() const {
	return hasComponent(ComponentType::ID);
}
