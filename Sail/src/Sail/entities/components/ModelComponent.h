#pragma once
#include "Component.h"
class Model;

class ModelComponent : public Component<ModelComponent> {
public:
	ModelComponent(Model* model = nullptr)
		: m_model(model)
	{ }
	~ModelComponent() { }

	void setModel(Model* model) {
		m_model = model;
	}

	Model* getModel() const {
		return m_model;
	}

private:
	Model* m_model;
};