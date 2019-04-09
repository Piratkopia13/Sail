#pragma once

#include <unordered_map>
#include <memory>
#include "components/Component.h"

#define MOVE(x) std::move(x)

class Entity {
public:
	typedef std::unique_ptr<Entity> Ptr;
	static Ptr Create();
public:
	Entity();
	virtual ~Entity();

	template<typename T, typename... Targs>
	T* addComponent(Targs... args);
	template<typename T>
	T* getComponent();
	

private:
	std::unordered_map<int, Component::Ptr> m_components;
};

template<typename T, typename... Targs>
T* Entity::addComponent(Targs... args) {
	auto res = m_components.insert({ T::getStaticID(), std::make_unique<T>(args...) });
	if (!res.second) {
		Logger::Warning("Tried to add a duplicate component to an entity");
	}
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