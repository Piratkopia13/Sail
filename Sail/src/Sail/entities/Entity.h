#pragma once

#include <unordered_map>
#include <memory>
#include "components/Component.h"

class Entity {
public:
	typedef std::shared_ptr<Entity> SPtr;
	static SPtr Create(const std::string& name = "");
public:
	Entity(const std::string& name = "");
	virtual ~Entity();

	template<typename T, typename... Targs>
	std::shared_ptr<T> addComponent(Targs... args);
	template<typename T>
	bool removeComponent();
	bool removeComponentByID(int id);
	template<typename T>
	std::shared_ptr<T> getComponent();
	
	void setName(const std::string& name);
	const std::string& getName() const;

	void setIsBeingRendered(bool value);
	bool isBeingRendered() const;

	std::unordered_map<int, Component::SPtr>& getAllComponents();

private:
	std::unordered_map<int, Component::SPtr> m_components;
	std::string m_name;
	bool m_isBeingRendered;
};

template<typename T, typename... Targs>
std::shared_ptr<T> Entity::addComponent(Targs... args) {
	auto res = m_components.insert({ T::getStaticID(), std::make_shared<T>(args...) });
	if (!res.second) {
		Logger::Warning("Tried to add a duplicate component to an entity");
	}
	// Return pointer to the inserted component
	return std::static_pointer_cast<T>(res.first->second);
}

template<typename T>
bool Entity::removeComponent() {
	auto res = m_components.erase(T::getStaticID());
	// Return true if component was successfully removed
	return res != 0;
}

template<typename T>
std::shared_ptr<T> Entity::getComponent() {
	// If the following line causes compile errors, then a class 
	// deriving from component is missing public SAIL_COMPONENT macro
	auto it = m_components.find(T::getStaticID());
	if (it != m_components.end())
		return std::static_pointer_cast<T>(it->second);

	return nullptr;
}
