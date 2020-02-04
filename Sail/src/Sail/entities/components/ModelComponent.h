#pragma once

#include "Component.h"

class Model;

class ModelComponent : public Component {
public:
	SAIL_COMPONENT
	ModelComponent(Model* model);
	~ModelComponent() { }

	Model* getModel();

private:
	Model* m_model;
};