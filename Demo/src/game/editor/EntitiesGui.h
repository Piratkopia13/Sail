#pragma once

#include "Sail/gui/SailGuiWindow.h"
#include "Sail/entities/Entity.h"

class Component;

class EntitiesGui : public SailGuiWindow {
public:
    EntitiesGui();
    void render(Scene* scene);

    // Registering a component allows it to show up in the component list of an entity in the GUI
    template<typename T>
    void registerComponent() {
        auto name = std::string(typeid(T).name()).substr(6);
        m_componentNames.insert({ entt::type_info<T>::id(), name }); // substr removes "class " from the type name
        
        m_componentNameList.emplace_back(name);
    }

private:
	const char* m_materialNames[2] = { "PBR", "Phong" };

private:
	void addComponent(Entity& entity, const char* componentName);
	void removeComponent(Entity& entity, const char* componentName);
	void addMaterialComponent(Entity& entity, const char* materialName);

    void selectEntity(Entity::ID entity, Scene* scene);
    void listEntity(Entity& e, uint32_t* index, Entity::ID* pSelectedEntity, bool entityAddedThisFrame);

private:
    Entity::ID m_selectedEntityID;

	std::unordered_map<ENTT_ID_TYPE, std::string> m_componentNames;
    std::vector<std::string> m_componentNameList; // Built from the names of registered components

    std::function<Component* (Entity&, ENTT_ID_TYPE)> m_getComponentInstanceFromID;

};