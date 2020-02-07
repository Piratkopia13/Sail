#pragma once

#include "Component.h"
#include "Sail/graphics/geometry/Model.h"

class ModelComponent : public Component {
public:
	SAIL_COMPONENT
	ModelComponent(Model::SPtr model);
	~ModelComponent() { }

	Model::SPtr getModel();

	void renderEditorGui(SailGuiWindow* window) override;

private:
	Model::SPtr m_model;
};