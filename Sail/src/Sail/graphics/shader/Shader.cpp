#include "pch.h"
#include "Shader.h"

Shader::Shader(const std::string& filename) {
	shaderPipeline = std::unique_ptr<ShaderPipeline>(ShaderPipeline::Create(filename));
	shaderPipeline->compile();
}

Shader::~Shader() {}

ShaderPipeline* Shader::getPipeline() {
	return shaderPipeline.get();
}

void Shader::bind() {
	shaderPipeline->bind();
}
