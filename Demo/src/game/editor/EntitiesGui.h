#pragma once

#include "Sail/gui/SailGuiWindow.h"
#include "Sail/entities/Entity.h"

namespace AddableComponent {
    enum Type {
        MeshComponent = 0,
        TransformComponent,
        MaterialComponent,
        PointLightComponent,
        DirectionalLightComponent,
        NUM_COMPONENTS
    };
}
namespace AddableMaterial {
	enum Type {
		PBRMaterial = 0,
        PhongMaterial,
        TexturesMaterial,
        OutlineMaterial,
        NUM_MATERIALS
	};
}
class EntitiesGui : public SailGuiWindow {
public:
    EntitiesGui();
    void render(Scene* scene);

private:
    // Should match order of AddableComponent
	const char* m_componentNames[AddableComponent::NUM_COMPONENTS] = { "MeshComponent", "TransformComponent", "MaterialComponent", "PointLightComponent", "DirectionalLightComponent" };
	const char* m_materialNames[AddableMaterial::NUM_MATERIALS] = { "PBR", "Phong", "Textures", "Outline" };

private:
    void selectEntity(Entity::ID entity, Scene* scene);
	//void addComponent(AddableComponent::Type comp);
	//void addMaterialComponent(AddableMaterial::Type comp);

private:
    Entity::ID m_selectedEntityID;

    void listEntity(Entity& e, uint32_t* index, Entity::ID* pSelectedEntity, bool entityAddedThisFrame);
};