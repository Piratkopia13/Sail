#pragma once

#include "Material.h"
#include <functional>

class Shader;

class DeferredShadingPassMaterial : public Material {
public:
	DeferredShadingPassMaterial();
	~DeferredShadingPassMaterial();

	virtual void* getData() override;
	unsigned int getDataSize() const override;
	Shader* getShader(Renderer::Type rendererType) const override;

	void setBindFunc(std::function<void(Shader*, Environment*, void*)> func);

private:
	std::function<void(Shader*, Environment*, void*)> m_bindFunc;

};