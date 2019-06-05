#include "pch.h"
#include "Shader.h"

Shader::Shader(const std::string& filename) 
	: m_finished(false)
{
	shaderPipeline = std::unique_ptr<ShaderPipeline>(ShaderPipeline::Create(filename));
	shaderPipeline->compile();
}

Shader::~Shader() {}

ShaderPipeline* Shader::getPipeline() {
	return shaderPipeline.get();
}

void Shader::bind() {
	if (!m_finished) {
		Logger::Error("A shader is trying to bind before it has finished its creation. Make sure to call shader::finish() at the end of the shader constructor.");
	}
	shaderPipeline->bind();
}

void Shader::finish() {
	shaderPipeline->finish();
	m_finished = true;
}
