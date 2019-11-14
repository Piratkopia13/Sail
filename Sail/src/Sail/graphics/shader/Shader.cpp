#include "pch.h"
#include "Shader.h"

Shader::Shader(const std::string& filename)
	: m_finished(false)
{
	shaderPipeline = ShaderPipeline::Create(filename);
	shaderPipeline->compile();
}

Shader::~Shader() {
	delete shaderPipeline;
}

ShaderPipeline* Shader::getPipeline() {
	return shaderPipeline;
}

void Shader::setWireframe(bool wireframe) {
	 shaderPipeline->setWireframe(wireframe);
}

void Shader::setCullMode(GraphicsAPI::Culling cullMode) {
	shaderPipeline->setCullMode(cullMode);
}

void Shader::bind() {
	SAIL_LOG_WARNING("Is shader::bind() used?"); // TODO: check if this method should exist, or if all binding is done directly through shaderPipeline
	if (!m_finished) {
		SAIL_LOG_ERROR("A shader is trying to bind before it has finished its creation. Make sure to call shader::finish() at the end of the shader constructor.");
	}
	shaderPipeline->bind(nullptr);
}

void Shader::finish() {
	shaderPipeline->finish();
	m_finished = true;
}
