#pragma once

#include "Component.h"
#include "Sail/graphics/material/Material.h"

class PhongMaterial;
class PBRMaterial;

class MaterialComponent : public Component {
public:
	SAIL_COMPONENT
	MaterialComponent(Material::Type type);
	~MaterialComponent() { }

	Material* get() {
		return m_material.get();
	}

	void renderEditorGui(SailGuiWindow* window) override;

private:
	void renderPhongMaterialGui(SailGuiWindow* window, PhongMaterial* material);
	void renderPBRMaterialGui(SailGuiWindow* window, PBRMaterial* material);

private:
	const LPCWSTR m_textureFilter;
	Material::SPtr m_material;
};