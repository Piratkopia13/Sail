#pragma once

#include "Sail/graphics/shader/Shader.h"

class DX12VertexBuffer;
class DX12ConstantBuffer;
class DX12StructuredBuffer;

class ParticleComputeShader : public Shader {
public:
	ParticleComputeShader();
	~ParticleComputeShader();

	virtual const Shader::ComputeSettings* getComputeSettings() const override;

private:
	Shader::ComputeSettings m_settings;
};
