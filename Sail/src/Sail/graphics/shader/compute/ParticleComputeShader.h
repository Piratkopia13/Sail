#pragma once

#include "Sail/graphics/shader/Shader.h"

class DX12VertexBuffer;
class DX12ConstantBuffer;
class DX12StructuredBuffer;

class ParticleComputeShader : public Shader {
public:
	class Input : public Shader::ComputeShaderInput {
	public:
		DX12ConstantBuffer* inputCB;
	};
	class Output : public Shader::ComputeShaderOutput {
	public:
		DX12VertexBuffer* outputVB;
		//DX12StructuredBuffer* outputSB;
	};

public:
	ParticleComputeShader();
	~ParticleComputeShader();

	virtual const Shader::ComputeSettings* getComputeSettings() const override;
	virtual std::pair<std::string, void*> getComputeInputForIndex(Shader::ComputeShaderInput& input, unsigned int index) override;
	virtual Shader::ComputeShaderOutput* getComputeOutput() override;

private:
	Shader::ComputeSettings m_settings;
	Output m_output;

};
