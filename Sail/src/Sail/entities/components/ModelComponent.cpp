#include "pch.h"
#include "ModelComponent.h"

ModelComponent::ModelComponent(Model* model) 
	: m_model(model) 
{ }

Model* ModelComponent::getModel() {
	return m_model;
}
