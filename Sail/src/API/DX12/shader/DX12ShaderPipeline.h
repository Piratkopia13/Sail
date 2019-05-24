#pragma once

#include "Sail/api/shader/ShaderPipeline.h"

class DX12ShaderPipeline : public ShaderPipeline {
public:
	DX12ShaderPipeline(const std::string& filename);
	~DX12ShaderPipeline();

	virtual void bind() override;
	virtual void* compileShader(const std::string& source, ShaderComponent::BIND_SHADER shaderType) override;
	virtual void setTexture2D(const std::string& name, void* handle) override;

};