#pragma once

#include "Material.h"
#include <functional>

class Shader;

class CustomMaterial : public Material {

public:
	CustomMaterial();
	~CustomMaterial();

	virtual void bind(Shader* shader, Environment* environment, void* cmdList = nullptr) override;

	void setBindFunc(std::function<void(Shader*, Environment*, void*)> func);

private:
	std::function<void(Shader*, Environment*, void*)> m_bindFunc;

};