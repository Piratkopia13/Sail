#pragma once

#include "Component.h"
class Model;

class ModelComponent : public Component {
public:
	SAIL_COMPONENT
	/*static int getStaticID() {
		return 1;
	}*/
	ModelComponent(Model* model)
		: m_model(model)
	{ }
	~ModelComponent() { }

	Model* getModel() {
		return m_model;
	}

	void renderEditorGui(SailGuiWindow* window) override;

private:
	Model* m_model;
};