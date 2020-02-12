#pragma once

#include "Sail/api/shader/Shader.h"

class GenerateMipsComputeShader : public Shader {
public:
	GenerateMipsComputeShader();
	~GenerateMipsComputeShader();

	virtual const Shader::ComputeSettings* getComputeSettings() const override;

private:
	bool m_clippingPlaneHasChanged;
	Shader::ComputeSettings m_settings;

};
