#pragma once

#include <entt.hpp>
#include "../graphics/Scene.h"

class Entity {
public:
	// Entity identifier
	// This is simply entt::entity behind the scenes
	// but has a bool operator overload for testing if the entity is null
	struct ID {
		entt::entity id = entt::null;

		ID(entt::entity id = entt::null) : id(id) { }
		operator bool() {
			return id != entt::null;
		}
		operator entt::entity() {
			return id;
		}
		operator entt::entity() const {
			return id;
		}
	};


public:
	Entity() = default;
	Entity(const Entity::ID& handle, Scene* scene);

	template<typename Component, typename... Args>
	Component& addComponent(Args&&... args) {
		return m_scene->getEnttRegistry().emplace<Component>(m_handle, std::forward<Args>(args)...);
	}

	template<typename Component, typename... Args>
	auto& addOrReplaceComponent(Args&&... args) {
		return m_scene->getEnttRegistry().emplace_or_replace<Component>(m_handle, std::forward<Args>(args)...);
	}

	template<typename... Component>
	bool hasComponent() const {
		return m_scene->getEnttRegistry().has<Component...>(m_handle);
	}

	template<typename Component>
	Component& getComponent() const {
		return m_scene->getEnttRegistry().get<Component>(m_handle);
	}

	template<typename Component>
	Component* tryGetComponent() const {
		return m_scene->getEnttRegistry().try_get<Component>(m_handle);
	}

	template<typename... Component>
	void removeComponent() {
		m_scene->getEnttRegistry().remove<Component...>(m_handle);
	}

	std::string getName() const;
	void setName(const std::string& name);

	void addChild(Entity& child);
	void removeChild(Entity& child);

	bool isBeingRendered() const;
	void setIsBeingRendered(bool isBeingRendered);

	bool isSelected() const;
	void setIsSelected(bool isSelected);

	uint32_t size() const;

	Scene* getScene();

	operator bool() {
		return m_handle.id != entt::null;
	}

	bool operator==(const Entity::ID& id) {
		return m_handle.id == id;
	}

	operator Entity::ID() {
		return m_handle;
	}

	operator entt::entity() {
		return m_handle.id;
	}

private:
	Entity::ID m_handle = {};
	Scene* m_scene = nullptr;

};
