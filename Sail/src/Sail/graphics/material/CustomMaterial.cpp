#include "pch.h"
#include "CustomMaterial.h"

CustomMaterial::CustomMaterial() 
	: Material(Type::CUSTOM)
{ }

CustomMaterial::~CustomMaterial() { }

void CustomMaterial::bind(Shader* shader, Environment* environment, void* cmdList) {
	m_bindFunc(shader, environment, cmdList);
}

void CustomMaterial::setBindFunc(std::function<void(Shader*, Environment*, void*)> func) {
	m_bindFunc = func;
}
