#pragma once

#include "Sail/graphics/shader/Shader.h"

class AnimationUpdateComputeShader : public Shader {
public:
	class Input : public Shader::ComputeShaderInput {
	public:
	};
	class Output : public Shader::ComputeShaderOutput {
	public:
	};

public:
	AnimationUpdateComputeShader();
	~AnimationUpdateComputeShader();

	virtual const Shader::ComputeSettings* getComputeSettings() const override;
	virtual std::pair<std::string, void*> getComputeInputForIndex(Shader::ComputeShaderInput& input, unsigned int index) override;
	virtual RenderableTexture* getComputeOutputForIndex(Shader::ComputeShaderOutput& output, unsigned int index) override;
	virtual Shader::ComputeShaderOutput* getComputeOutput() override;

private:
	Shader::ComputeSettings m_settings;
	Output m_output;

};
