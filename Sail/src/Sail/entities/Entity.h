#pragma once

#include <unordered_map>
#include <memory>
#include "components/Component.h"

class Entity {
public:
	typedef std::shared_ptr<Entity> SPtr;
	static SPtr Create(const std::string& name = "");
public:
	virtual ~Entity();

	template<typename T, typename... Targs>
	T* addComponent(Targs... args);
	template<typename T>
	bool removeComponent();
	template<typename T>
	T* getComponent();
	
	void setName(const std::string& name);
	const std::string& getName() const;
	Entity(const std::string& name = "");

	std::unordered_map<int, Component::Ptr>& getAllComponents();

private:
	std::unordered_map<int, Component::Ptr> m_components;
	std::string m_name;
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
bool Entity::removeComponent() {
	auto res = m_components.erase(T::getStaticID());
	// Return true if component was successfully removed
	return res != 0;
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
