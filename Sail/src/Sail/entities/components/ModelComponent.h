#pragma once

#include "Component.h"

class Model;
class PhongMaterial;
class PBRMaterial;

class ModelComponent : public Component {
public:
	SAIL_COMPONENT
	ModelComponent(Model* model)
		: m_model(model)
	{ }
	~ModelComponent() { }

	Model* getModel() {
		return m_model;
	}

	void renderEditorGui(SailGuiWindow* window) override;

private:
	void renderPhongMaterialGui(SailGuiWindow* window, PhongMaterial* material);
	void renderPBRMaterialGui(SailGuiWindow* window, PBRMaterial* material);

private:
	Model* m_model;
};