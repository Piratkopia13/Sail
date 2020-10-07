#include "pch.h"
#include "DeferredShadingPassMaterial.h"
#include "Sail/Application.h"

DeferredShadingPassMaterial::DeferredShadingPassMaterial() 
	: Material(Type::CUSTOM)
{ }

DeferredShadingPassMaterial::~DeferredShadingPassMaterial() { }

//void DeferredShadingPassMaterial::bind(Shader* shader, Environment* environment, void* cmdList) {
//	m_bindFunc(shader, environment, cmdList);
//}

void* DeferredShadingPassMaterial::getData() {
	return nullptr;
}

unsigned int DeferredShadingPassMaterial::getDataSize() const {
	return 0;
}


Shader* DeferredShadingPassMaterial::getShader(Renderer::Type rendererType) const {
	auto& resman = Application::getInstance()->getResourceManager();
	switch (rendererType) {
	case Renderer::DEFERRED:
		return &resman.getShaderSet(Shaders::DeferredShadingPassShader);
		break;
	default:
		return nullptr;
		break;
	}
}

void DeferredShadingPassMaterial::setBindFunc(std::function<void(Shader*, Environment*, void*)> func) {
	m_bindFunc = func;
}