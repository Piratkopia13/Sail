#include "pch.h"
#include "DX12ShaderPipeline.h"

ShaderPipeline* ShaderPipeline::Create(const std::string& filename) {
	return new DX12ShaderPipeline(filename);
}

DX12ShaderPipeline::DX12ShaderPipeline(const std::string& filename) 
	: ShaderPipeline(filename)
{

}

DX12ShaderPipeline::~DX12ShaderPipeline() {

}

void DX12ShaderPipeline::bind() {
	throw std::logic_error("The method or operation is not implemented.");
}

void* DX12ShaderPipeline::compileShader(const std::string& source, ShaderComponent::BIND_SHADER shaderType) {
	throw std::logic_error("The method or operation is not implemented.");
}

void DX12ShaderPipeline::setTexture2D(const std::string& name, void* handle) {
	throw std::logic_error("The method or operation is not implemented.");
}
